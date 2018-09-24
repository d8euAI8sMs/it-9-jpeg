#pragma once

#include <util/common/geom/point.h>
#include <util/common/math/vec.h>
#include <util/common/plot/plot.h>
#include <util/common/math/fuzzy.h>

#include <cstdint>
#include <vector>
#include <map>
#include <array>

#include <omp.h>

namespace model
{

    using byte_t = std::uint8_t;
    using pixel_t = std::uint32_t;

    struct m8
    {
        std::array < std::array < double, 8 >, 8 > m;
    };

    struct img_header
    {
        size_t w, h;
    };

    struct rle
    {
        byte_t n;
        byte_t v;
    };

    struct rle_data
    {
        img_header header;
        std::vector < rle > data;
        size_t size() const { return data.size() * 2; }
    };

    /*****************************************************/
    /*                     params                        */
    /*****************************************************/

    namespace consts
    {
        static const math::m3<> rgb2yuv = {
            { 0.299,      0.587,     0.114    },
            { -0.168736, -0.331264,  0.5      },
            { 0.5,       -0.418688, -0.081312 }
        };
        static const math::m3<> yuv2rgb = {
            { 1,   0,         1.402    },
            { 1,  -0.344136, -0.714136 },
            { 1,   1.772,     0        }
        };
        static const double sqrt2tom1 = 0.70710678118654752440084436210485;
        static const double pi = 3.1415926535897932384626433832795;
        static const m8 QY = { { {
            { { 16,  11,  10,  16,  24,  40,  51,  61  } },
            { { 12,  12,  14,  19,  26,  58,  60,  55  } },
            { { 14,  13,  16,  24,  40,  57,  69,  56  } },
            { { 14,  17,  22,  29,  51,  87,  80,  62  } },
            { { 18,  22,  37,  56,  68,  109, 103, 77  } },
            { { 24,  35,  55,  64,  81,  104, 113, 92  } },
            { { 49,  64,  78,  87,  103, 121, 120, 101 } },
            { { 72,  92,  95,  98,  112, 100, 103, 99  } }
        } } };
        static const m8 QC = { { {
            { { 17, 18, 24, 47, 99, 99, 99, 99 } },
            { { 18, 21, 26, 66, 99, 99, 99, 99 } },
            { { 24, 26, 56, 99, 99, 99, 99, 99 } },
            { { 47, 66, 99, 99, 99, 99, 99, 99 } },
            { { 99, 99, 99, 99, 99, 99, 99, 99 } },
            { { 99, 99, 99, 99, 99, 99, 99, 99 } },
            { { 99, 99, 99, 99, 99, 99, 99, 99 } },
            { { 99, 99, 99, 99, 99, 99, 99, 99 } }
        } } };
    };

    namespace globs
    {
        static std::array < std::array < double, 8 >, 8 > costf;
        static std::array < std::pair < size_t, size_t >, 64 > zigzag;
        inline void init()
        {
            for (size_t i = 0; i < 8; ++i)
            for (size_t u = 0; u < 8; ++u)
            {
                costf[i][u] = ((u == 0) ? consts::sqrt2tom1 : 1) * std::cos((2 * i + 1) * u * consts::pi / 16) / 2;
            }

            size_t k = 0;
            for (size_t i = 0; i < 8; ++i)
            for (size_t j = 0; j < 8; ++j)
            {
                size_t u, v;
                if (i & 1) { u = i - j; v = j;     }
                else       { u = j;     v = i - j; }
                zigzag[k++] = { u, v };
                if (i == j) break;
            }
            for (size_t i = 1; i < 8; ++i)
            for (size_t j = 0; j < 8; ++j)
            {
                size_t u, v;
                if (i & 1) { u = i + j; v = 7 - j; }
                else       { u = 7 - j; v = i + j; }
                zigzag[k++] = { u, v };
                if (i + j == 7) break;
            }
        }
    };

    /*****************************************************/
    /*                     util                          */
    /*****************************************************/

    inline m8 & costr(m8 & s)
    {
        m8 n;
        for (size_t u = 0; u < 8; ++u)
        for (size_t v = 0; v < 8; ++v)
        {
            double uv = 0;
            for (size_t i = 0; i < 8; ++i)
            for (size_t j = 0; j < 8; ++j)
            {
                uv += globs::costf[i][u] * globs::costf[j][v] * s.m[i][j];
            }
            n.m[u][v] = uv;
        }
        return (s = n);
    }

    inline m8 & costrr(m8 & s)
    {
        m8 n;
        for (size_t u = 0; u < 8; ++u)
        for (size_t v = 0; v < 8; ++v)
        {
            double uv = 0;
            for (size_t i = 0; i < 8; ++i)
            for (size_t j = 0; j < 8; ++j)
            {
                uv += globs::costf[u][i] * globs::costf[v][j] * s.m[i][j];
            }
            n.m[u][v] = byte_t(min(255, max(0, uv)));
        }
        return (s = n);
    }

    inline m8 & appq(m8 & s, const m8 & q)
    {
        for (size_t u = 0; u < 8; ++u)
        for (size_t v = 0; v < 8; ++v)
        {
            s.m[u][v] = byte_t(min(255, max(0, s.m[u][v] / q.m[u][v])));
        }
        return s;
    }

    inline m8 & appqr(m8 & s, const m8 & q)
    {
        for (size_t u = 0; u < 8; ++u)
        for (size_t v = 0; v < 8; ++v)
        {
            s.m[u][v] = s.m[u][v] * q.m[u][v];
        }
        return s;
    }

    inline void rle_encode(m8 & m, std::vector < rle > & rles)
    {
        rle cur = { 0, 0 };
        for (size_t i = 0; i < 64; ++i)
        {
            auto uv = globs::zigzag[i];
            if (cur.v != m.m[uv.first][uv.second])
            {
                if (cur.n != 0)
                    rles.push_back(cur);
                cur.n = 1;
                cur.v = byte_t(m.m[uv.first][uv.second]);
            }
            else ++cur.n;
        }
        if (cur.n != 0) rles.push_back(cur);
    }

    inline void rle_decode(m8 & m, std::vector < rle > & rles)
    {
        rle cur = { 0, 0 };
        for (size_t i = 0; i < 64; ++i)
        {
            if (cur.n == 0)
            {
                cur = rles.back();
                rles.pop_back();
            }
            auto uv = globs::zigzag[64 - i - 1];
            m.m[uv.first][uv.second] = cur.v;
            --cur.n;
        }
    }

    /*****************************************************/
    /*                     data                          */
    /*****************************************************/

    union color_t
    {
        struct 
        {
            byte_t r;
            byte_t g;
            byte_t b;
            byte_t a;
        };
        struct 
        {
            byte_t y;
            byte_t cb;
            byte_t cr;
            byte_t a;
        };
        pixel_t c;
        
        color_t & to_yuv()
        {
            auto n = consts::rgb2yuv * math::v3<>(r,g,b) + math::v3<>(0, 128, 128);
            y  = byte_t(min(255, max(0, n.x)));
            cb = byte_t(min(255, max(0, n.y)));
            cr = byte_t(min(255, max(0, n.z)));
            return *this;
        }
        color_t yuv() { color_t n(*this); n.to_yuv(); return n; }
        
        color_t & to_rgba()
        {
            auto n = consts::yuv2rgb * (math::v3<>(y,cb,cr) - math::v3<>(0, 128, 128));
            r = byte_t(min(255, max(0, n.x)));
            g = byte_t(min(255, max(0, n.y)));
            b = byte_t(min(255, max(0, n.z)));
            return *this;
        }
        color_t rgba() { color_t n(*this); n.to_rgba(); return n; }

        color_t bgra() const
        {
            color_t n; n.c = (a << 24) | (r << 16) | (g << 8) | b; return n;
        }

        byte_t operator[] (size_t i) const { return (c >> (i * 8)) & 0xff; }
    };

    class bitmap
    {
    public:
        img_header header;
        std::vector < std::vector < color_t > > pixels;
    public:
        void from_cbitmap(CBitmap & bmp)
        {
            BITMAP bmpd;
            bmp.GetBitmap(&bmpd);
            header.w = bmpd.bmWidth; header.h = bmpd.bmHeight;
            CDC dc; dc.CreateCompatibleDC(nullptr);
            dc.SelectObject(&bmp);
            pixels.resize(header.h);
            for (size_t i = 0; i < header.h; ++i)
            {
                pixels[i].resize(header.w);
                for (size_t j = 0; j < header.w; ++j)
                    pixels[i][j].c = dc.GetPixel(j, i);
            }
        }
        void to_cbitmap(CBitmap & bmp)
        {
            std::vector < pixel_t > buf(header.h * header.w);
            for (size_t i = 0; i < header.h; ++i)
            for (size_t j = 0; j < header.w; ++j)
            {
                buf[header.w * i + j] = pixels[i][j].bgra().c;
            }
            bmp.DeleteObject();
            bmp.CreateBitmap(header.w, header.h, 1, 32, (LPCVOID) buf.data());
            bmp.SetBitmapDimension(header.w, header.h);
        }
    public:
        void jpeg_compress(rle_data & d)
        {
            d.header = header;

            size_t n = (header.h + 7) / 8, m = (header.w + 7) / 8;

            for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < m; ++j)
            {
                auto m8bs = m8bs_at(i, j);
                auto y8  = costr(m8bs[0]);
                auto cb8 = costr(m8bs[1]);
                auto cr8 = costr(m8bs[2]);
                rle_encode(appq(y8,  consts::QY), d.data);
                rle_encode(appq(cb8, consts::QC), d.data);
                rle_encode(appq(cr8, consts::QC), d.data);
            }
        }
        void jpeg_decompress(rle_data & d)
        {
            header = d.header;
            pixels.clear();
            pixels.resize(header.h);
            for (size_t i = 0; i < header.h; ++i)
                pixels[i].resize(header.w);

            size_t n = (header.h + 7) / 8, m = (header.w + 7) / 8;

            rle_data jd = d; // copy to leave `d` untouched

            for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < m; ++j)
            {
                std::array < m8, 3 > m8bs;
                rle_decode(m8bs[2], jd.data);
                rle_decode(m8bs[1], jd.data);
                rle_decode(m8bs[0], jd.data);
                auto y8  = costrr(appqr(m8bs[0], consts::QY));
                auto cb8 = costrr(appqr(m8bs[1], consts::QC));
                auto cr8 = costrr(appqr(m8bs[2], consts::QC));
                m8bs_at(n - i - 1, m - j - 1, {{ y8, cb8, cr8 }});
            }
        }
        void rle_compress(rle_data & d)
        {
            d.header = header;
            rle cur = { 0, 0 };
            for (size_t c = 0; c < 3; ++c)
            {
                cur = { 0, 0 };
                for (size_t i = 0; i < header.h; ++i){
                for (size_t j = 0; j < header.w; ++j)
                {
                    size_t y = i, x = (i & 1) ? (header.w - j - 1) : j;
                    if (cur.v != pixels[y][x][c] || cur.n == 0xff)
                    {
                        if (cur.n != 0)
                            d.data.push_back(cur);
                        cur.n = 1;
                        cur.v = pixels[y][x][c];
                    }
                    else ++cur.n;
                }}
                if (cur.n != 0) d.data.push_back(cur);
            }
        }
        void rle_decompress(rle_data & _d)
        {
            header = _d.header;
            pixels.clear();
            pixels.resize(header.h);
            for (size_t i = 0; i < header.h; ++i)
                pixels[i].resize(header.w);

            rle_data d = _d;

            rle cur = { 0, 0 };
            for (size_t c = 0; c < 3; ++c)
            {
                for (size_t i = 0; i < header.h; ++i)
                for (size_t j = 0; j < header.w; ++j)
                {
                    size_t y = header.h - i - 1, x = (i & 1) ? j : (header.w - j - 1);
                    if (cur.n == 0)
                    {
                        cur = d.data.back();
                        d.data.pop_back();
                    }
                    pixels[y][x].c |= cur.v << ((2 - c) * 8);
                    --cur.n;
                }
            }
        }
        size_t size() const { return header.w * header.h * 3; /* without alpha */ }
    private:
        std::array < m8, 3 > m8bs_at(size_t i, size_t j)
        {
            std::array < m8, 3 > ms;
            size_t x = j * 8, y = i * 8;
            for (size_t a = 0; a < min(8, header.w - x); ++a)
            for (size_t b = 0; b < min(8, header.h - y); ++b)
            {
                auto p = pixels[y + b][x + a].yuv();
                ms[0].m[b][a] = p.y;
                ms[1].m[b][a] = p.cb;
                ms[2].m[b][a] = p.cr;
            }
            for (size_t a = min(8, header.w - x); a < 8; ++a)
            for (size_t b = 0; b < 8; ++b)
            {
                ms[0].m[b][a] = ms[0].m[b][min(8, header.w - x) - 1];
                ms[1].m[b][a] = ms[1].m[b][min(8, header.w - x) - 1];
                ms[2].m[b][a] = ms[2].m[b][min(8, header.w - x) - 1];
            }
            for (size_t a = 0; a < 8; ++a)
            for (size_t b = min(8, header.h - y); b < 8; ++b)
            {
                ms[0].m[b][a] = ms[0].m[min(8, header.h - y) - 1][a];
                ms[1].m[b][a] = ms[1].m[min(8, header.h - y) - 1][a];
                ms[2].m[b][a] = ms[2].m[min(8, header.h - y) - 1][a];
            }
            return ms;
        }
        void m8bs_at(size_t i, size_t j, const std::array < m8, 3 > & ms)
        {
            size_t x = j * 8, y = i * 8;
            for (size_t a = 0; a < min(8, header.w - x); ++a)
            for (size_t b = 0; b < min(8, header.h - y); ++b)
            {
                pixels[y + b][x + a].y  = byte_t(ms[0].m[b][a]);
                pixels[y + b][x + a].cb = byte_t(ms[1].m[b][a]);
                pixels[y + b][x + a].cr = byte_t(ms[2].m[b][a]);
                pixels[y + b][x + a].to_rgba();
            }
        }
    };

    /*****************************************************/
    /*                     drawing                       */
    /*****************************************************/

    struct bmp_data
    {
        bitmap bmp;
        CBitmap cbmp;
    };

    struct model_data
    {
        bmp_data src;
        bmp_data dst;
        rle_data rsrc;
        rle_data rdst;
    };

    inline plot::drawable::ptr_t make_bmp_plot(bmp_data & b)
    {
        return plot::custom_drawable::create([&b] (CDC & dc, const plot::viewport & vp)
        {
            if (!b.bmp.header.h) return;
            CDC memDC; memDC.CreateCompatibleDC(&dc);
            memDC.SelectObject(&b.cbmp);
            auto wh = b.cbmp.GetBitmapDimension();
            dc.SetStretchBltMode(HALFTONE);
            dc.StretchBlt(vp.screen.xmin, vp.screen.ymin,
                          vp.screen.width(), vp.screen.height(),
                          &memDC, 0, 0, wh.cx, wh.cy, SRCCOPY);
        });
    }
}