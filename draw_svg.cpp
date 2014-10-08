#include "agg_color_rgba.h"

#include "draw_svg.h"

const char *svg_path_property_name[] = {"stroke-dasharray", "marker-start", "marker-mid", "marker-end"};

void format_rgb(char rgbstr[], agg::rgba8 c)
{
    sprintf(rgbstr, "#%02X%02X%02X", (int)c.r, (int)c.g, (int)c.b);
}

static void append_properties(str& s, svg_property_list* properties)
{
    for (svg_property_list* p = properties; p; p = p->next())
    {
        svg_property_item& item = p->content();
        const char* name = svg_path_property_name[item.key];
        s.printf_add(";%s:%s", name, item.value);
    }
}

static void property_append_alpha(str& s, const char* prop, agg::rgba8 c)
{
    if (c.a < 255) {
        double alpha = (double)c.a / 255;
        s.printf_add(";%s:%g", prop, alpha);
    }
}

static str gen_path_element(str& content, str& style, int id)
{
    str s = str::print("<path d=\"%s\" ", content.cstr());
    if (id >= 0)
        s.printf_add("id=\"path%i\" ", id);
    s.printf_add("style=\"%s\" />", style.cstr());
    return s;
}

str svg_stroke_path(str& path_coords, double width, int id, agg::rgba8 c,
                    svg_property_list* properties)
{
    char rgbstr[8];
    format_rgb(rgbstr, c);

    str s = str::print("fill:none;stroke:%s;"
                       "stroke-width:%g;stroke-linecap:butt;"
                       "stroke-linejoin:miter",
                       rgbstr, width);

    property_append_alpha(s, "stroke-opacity", c);
    append_properties(s, properties);

    return gen_path_element(path_coords, s, id);
}

str svg_marker_path(str& path_coords, double sw, int id, svg_property_list* properties)
{
    str s = str::print("fill:none;stroke:none;stroke-width:%g", sw);
    append_properties(s, properties);
    return gen_path_element(path_coords, s, id);
}

str svg_fill_path(str& path_coords, int id, agg::rgba8 c,
                  svg_property_list* properties)
{
    char rgbstr[8];
    format_rgb(rgbstr, c);
    str s = str::print("fill:%s;stroke:none", rgbstr);
    property_append_alpha(s, "fill-opacity", c);
    append_properties(s, properties);
    return gen_path_element(path_coords, s, id);
}
