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

str svg_stroke_path(str& path_coords, double width, int id, agg::rgba8 c,
		    svg_property_list* properties)
{
  char rgbstr[8];
  format_rgb(rgbstr, c);

  str s = str::print("<path d=\"%s\" "
		     "id=\"path%i\" "
		     "style=\"fill:none;stroke:%s;"
		     "stroke-width:%gpx;stroke-linecap:butt;"
		     "stroke-linejoin:miter", 
		     path_coords.cstr(), id, rgbstr, width);

  if (c.a < 255) {
    double alpha = (double)c.a / 255;
    s.printf_add(";stroke-opacity:%g", alpha);
  }

  append_properties(s, properties);
  s.append("\" />");

  return s;
}

str svg_marker_path(str& path_coords, double sw, int id, svg_property_list* properties)
{
  str s = str::print("<path d=\"%s\" "
		     "id=\"path%i\" "
		     "style=\"fill:none;stroke:none;stroke-width:%g",
		     path_coords.cstr(), id, sw);

  append_properties(s, properties);
  s.append("\" />");

  return s;
}

str svg_fill_path(str& path_coords, int id, agg::rgba8 c,
		  svg_property_list* properties)
{
  char rgbstr[8];
  format_rgb(rgbstr, c);

  str s = str::print("<path d=\"%s\" "
		     "id=\"path%i\" "
		     "style=\"fill:%s;stroke:none", 
		     path_coords.cstr(), id, rgbstr);

  if (c.a < 255) {
    double alpha = (double)c.a / 255;
    s.printf_add(";fill-opacity:%g", alpha);
  }
  
  append_properties(s, properties);
  s.append("\" />");

  return s;
}
