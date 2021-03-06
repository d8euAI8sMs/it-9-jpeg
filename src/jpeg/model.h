#pragma once

#include <util/common/geom/point.h>
#include <util/common/math/vec.h>
#include <util/common/plot/plot.h>
#include <util/common/math/fuzzy.h>

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <array>

#include <omp.h>

namespace model
{

    using byte_t = std::uint8_t;
    using sbyte_t = std::int8_t;
    using pixel_t = std::uint32_t;

    struct m8
    {
        std::array < std::array < double, 8 >, 8 > m;
    };

    enum class img_type { bmp, rle, huff, jpeg, jpeg_huff };

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

    struct jpeg_data : public rle_data { double quality; };

    struct huffman_code
    {
        size_t code;
        byte_t bits;

        bool operator == (const huffman_code & o) const
        {
            return (code == o.code) && (bits == o.bits);
        }
        bool operator != (const huffman_code & o) const
        {
            return (code != o.code) || (bits != o.bits);
        }
        bool operator < (const huffman_code & o) const
        {
            return (bits < o.bits) || (bits == o.bits) && (code < o.code);
        }
    };

    struct huffman_table
    {
        std::map < byte_t, huffman_code > index;
        std::map < huffman_code, byte_t > rev_index;
        size_t min_bits;

        void clear()
        {
            index.clear();
            rev_index.clear();
            min_bits = 0;
        }
    };

    struct huffman_data
    {
        img_header header;

        huffman_table table;

        size_t size_bits;
        std::vector < byte_t > data;

        void clear()
        {
            data.clear();
            size_bits = 0;
        }
        void put_symbol(byte_t s)
        {
            put_bits(table.index[s]);
        }
        byte_t pop_symbol()
        {
            size_t code = 0, len = table.min_bits;
            for (size_t i = 1; i < table.min_bits; ++i)
                code = (code << 1) | pop_bit();
            for (;;)
            {
                code = (code << 1) | pop_bit();
                auto it = table.rev_index.find({ code, len });
                if (it != std::end(table.rev_index))
                {
                    return it->second;
                }
                ++len;
            }
        }
        void put_bits(huffman_code bits)
        {
            data.resize((size_bits + 7) / 8 + 4);
            for (size_t i = 0; i < bits.bits; ++i)
            {
                put_bit(bits.code & 1);
                bits.code >>= 1;
            }
        }
        void put_bit(byte_t v)
        {
            if (v)
            {
                size_t defect = 8 - (size_bits % 8) - 1;
                data[size_bits / 8] |= (v << defect);
            }
            ++size_bits;
        }
        byte_t pop_bit()
        {
            --size_bits;
            size_t defect = 8 - (size_bits % 8) - 1;
            byte_t t = (data[size_bits / 8] & (1 << defect)) >> defect;
            return t;
        }
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
        extern std::array < std::array < double, 8 >, 8 > costf;
        extern std::array < std::pair < size_t, size_t >, 64 > zigzag;
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

    inline m8 make_quality_matrix(const m8 & src, double q, double k = 200)
    {
        m8 r;
        for (size_t i = 0; i < 8; ++i)
        for (size_t j = 0; j < 8; ++j)
            r.m[i][j] = src.m[i][j] * (31 - 60 * q);
        r.m[0][0] = src.m[0][0];
        return r;
    }

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
            s.m[u][v] = byte_t((sbyte_t)min(127, max(-127, s.m[u][v] / q.m[u][v])));
        }
        return s;
    }

    inline m8 & appqr(m8 & s, const m8 & q)
    {
        for (size_t u = 0; u < 8; ++u)
        for (size_t v = 0; v < 8; ++v)
        {
            s.m[u][v] = ((sbyte_t)byte_t(s.m[u][v])) * q.m[u][v];
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

    inline void make_huffman_table(huffman_table & tbl, std::map < byte_t, size_t > w)
    {
        struct tree { tree *l; tree *r; tree *p; bool leaf; byte_t b; size_t v; };
        auto cmp = [] (tree * t1, tree * t2) { return t1->v > t2->v; };
        std::vector < tree * > trees;
        std::vector < tree * > list;
        for each (auto & p in w)
            list.push_back(new tree { nullptr, nullptr, nullptr, true, p.first, p.second });
        while (list.size() != 1)
        {
            std::sort(std::begin(list), std::end(list), cmp);
            tree * t1 = *list.rbegin(); list.resize(list.size() - 1);
            tree * t2 = *list.rbegin(); list.resize(list.size() - 1);
            tree * t3 = new tree{ t1->p > t2->p ? t1 : t2, t1->p > t2->p ? t2 : t1, nullptr, false, 0, t1->v + t2->v };
            t1->p = t3; t2->p = t3;
            trees.push_back(t1); trees.push_back(t2);
            list.push_back(t3);
        }
        trees.push_back(list.back());
        tbl.clear();
        tbl.min_bits = 0xff;
        for (size_t i = 0; i < trees.size(); ++i)
        {
            if (!trees[i]->leaf) continue;
            size_t code = 0;
            tree * t = trees[i];
            byte_t b = t->b;
            size_t d = 0;
            while (t->p)
            {
                if (t->p->l == t) code |= 1 << d;
                ++d;
                t = t->p;
            }
            tbl.index.emplace(b, huffman_code { code, d });
            tbl.rev_index.emplace(huffman_code { code, d }, b);
            tbl.min_bits = min(tbl.min_bits, d);
        }
    }

    inline std::map < byte_t, size_t > get_rle_stat(rle_data & d)
    {
        std::map < byte_t, size_t > stat;
        for (size_t i = 0; i < d.data.size(); ++i)
        {
            ++stat[d.data[i].n];
            ++stat[d.data[i].v];
        }
        return stat;
    }

    inline void huffman_compress(huffman_data & hd, rle_data & rd)
    {
        hd.header = rd.header;
        hd.clear();
        for (size_t i = 0; i < rd.data.size(); ++i)
        {
            hd.put_symbol(rd.data[i].n);
            hd.put_symbol(rd.data[i].v);
        }
    }

    inline void huffman_decompress(huffman_data & hd, rle_data & rd)
    {
        huffman_data d = hd; // copy to not affect `hd`
        rd.header = hd.header;
        rd.data.clear();
        while (d.size_bits)
        {
            rd.data.emplace_back();
            rd.data.back().v = d.pop_symbol();
            rd.data.back().n = d.pop_symbol();
        }
        std::reverse(std::begin(rd.data), std::end(rd.data));
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
        void jpeg_compress(jpeg_data & d, double q = 0.5)
        {
            d.header = header;
            d.quality = q;

            d.data.clear();

            auto qy = make_quality_matrix(consts::QY, q);
            auto qc = make_quality_matrix(consts::QC, q);

            size_t n = (header.h + 7) / 8, m = (header.w + 7) / 8;

            for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < m; ++j)
            {
                auto m8bs = m8bs_at(i, j);
                auto y8  = costr(m8bs[0]);
                auto cb8 = costr(m8bs[1]);
                auto cr8 = costr(m8bs[2]);
                rle_encode(appq(y8,  qy), d.data);
                rle_encode(appq(cb8, qc), d.data);
                rle_encode(appq(cr8, qc), d.data);
            }
        }
        void jpeg_decompress(jpeg_data & d)
        {
            header = d.header;
            pixels.clear();
            pixels.resize(header.h);
            for (size_t i = 0; i < header.h; ++i)
                pixels[i].resize(header.w);

            auto qy = make_quality_matrix(consts::QY, d.quality);
            auto qc = make_quality_matrix(consts::QC, d.quality);

            size_t n = (header.h + 7) / 8, m = (header.w + 7) / 8;

            rle_data jd = d; // copy to leave `d` untouched

            for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < m; ++j)
            {
                std::array < m8, 3 > m8bs;
                rle_decode(m8bs[2], jd.data);
                rle_decode(m8bs[1], jd.data);
                rle_decode(m8bs[0], jd.data);
                auto y8  = costrr(appqr(m8bs[0], qy));
                auto cb8 = costrr(appqr(m8bs[1], qc));
                auto cr8 = costrr(appqr(m8bs[2], qc));
                m8bs_at(n - i - 1, m - j - 1, {{ y8, cb8, cr8 }});
            }
        }
        void rle_compress(rle_data & d)
        {
            d.header = header;
            d.data.clear();
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
        jpeg_data rdst;
        huffman_data hsrc;
        huffman_data hdst;
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

    using points_t = std::vector < geom::point2d_t > ;

    struct plot_data
    {
        util::ptr_t < points_t > data;
        plot::list_drawable < points_t > :: ptr_t plot;
    };

    struct plot_config
    {
        plot::world_t::ptr_t world;
        plot::auto_viewport < points_t > :: ptr_t autoworld;
    };

    inline plot_data make_plot_data
    (
        plot::palette::pen_ptr pen = plot::palette::pen(0xffffff),
        plot::list_data_format data_format = plot::list_data_format::chain
    )
    {
        plot_data pd;
        pd.data = util::create < points_t > ();
        pd.plot = plot::list_drawable < points_t > :: create
        (
            plot::make_data_source(pd.data),
            nullptr, // no point painter
            pen
        );
        pd.plot->data_format = data_format;
        return pd;
    }

    inline plot::drawable::ptr_t make_root_drawable
    (
        const plot_config & p,
        std::vector < plot::drawable::ptr_t > layers
    )
    {
        using namespace plot;

        return viewporter::create(
            tick_drawable::create(
                layer_drawable::create(layers),
                const_n_tick_factory<axe::x>::create(
                    make_simple_tick_formatter(6, 8),
                    0,
                    5
                ),
                const_n_tick_factory<axe::y>::create(
                    make_simple_tick_formatter(6, 8),
                    0,
                    5
                ),
                palette::pen(RGB(80, 80, 80)),
                RGB(200, 200, 200)
            ),
            make_viewport_mapper(make_world_mapper < points_t > (p.autoworld))
        );
    }

    inline plot_config make_plot_config()
    {
        plot_config cfg;
        cfg.world = plot::world_t::create();
        cfg.autoworld = plot::min_max_auto_viewport < points_t > :: create();
        return cfg;
    }
}