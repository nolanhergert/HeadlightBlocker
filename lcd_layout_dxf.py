'''
import ezdxf

doc = ezdxf.new()
msp = doc.modelspace()
doc.layers.add(name="TopLayer")

points = [(0, 0), (3, 0), (6, 3), (6, 6)]
msp.add_lwpolyline(points, dxfattribs={'layer': 'TopLayer'})

doc.saveas("lwpolyline1.dxf")
doc.saveas("lwpolyline1.png")
'''



import ezdxf
from ezdxf.addons.drawing import Frontend, RenderContext, svg, layout
import math

ezdxf.addons.drawing.properties.MODEL_SPACE_BG_COLOR = "#FFFFFF"
TRANSPARENCY = 0.75

# units in mm 
trace_gap = .25
pixel_spacing = 1.0 
grid_width = 5
grid_height = 4

# Flat top hexagon
#   ______
#  /      \
# /        \
# \        /
#  \______/
#
# (0, 0) is bottom left of screen, x increases to the right, y increases upwards. (math coordinates)

hexagon_half_height = (pixel_spacing- trace_gap) / 2 
hexagon_side_length = hexagon_half_height / math.cos(math.radians(30))
hexagon_half_width =  hexagon_side_length 


angled_gap_x = math.cos(math.radians(60)) * trace_gap
angled_gap_y = math.sin(math.radians(60)) * trace_gap

#def wayfarer_outline(msp):

def hexagon_horizontal_line(x,y):
    polyline = [(x, y)]
    # Start downwards always for now
    polarity = -1
    while x < grid_width:
        x += hexagon_half_width/2 + angled_gap_x
        y += (hexagon_half_height + angled_gap_y)*polarity
        polyline.append((x, y))
        x += hexagon_side_length
        polyline.append((x, y))
        polarity *= -1

    return polyline

def hexagons_horizontal_traces(doc):
    msp = doc.modelspace()
    layer = doc.layers.add(name="BottomLayer")
    layer.transparency = TRANSPARENCY
    
    
    x = -angled_gap_x
    y = hexagon_half_height + angled_gap_y

    while y < grid_height:
        polyline = []
        polyline.extend(hexagon_horizontal_line(x, y))
        msp.add_lwpolyline(polyline, dxfattribs={'layer': 'BottomLayer', 'color': 1})  # Color 1 corresponds to red in ACI
        polyline = []
        foo = hexagon_horizontal_line(x + hexagon_side_length, y)
        foo.reverse()   
        polyline.extend(foo)
        msp.add_lwpolyline(polyline, dxfattribs={'layer': 'BottomLayer', 'color': 3})

        # optional: keep a closed lwpolyline for visibility
        #msp.add_lwpolyline(polyline, dxfattribs={'closed': True, 'layer': 'BottomLayer', 'color': 4})
        # add a solid hatch to fill the polygohexagons_horizontal_tracesn defined by `polyline`
        hatch = msp.add_hatch()
        hatch.set_solid_fill(1)               # choose ACI color index
        #hatch.paths.add_polyline_path(polyline, is_closed=True)
        hatch.dxf.layer = 'BottomLayer'
        

        y += hexagon_half_height*2 + trace_gap



def hexagon_vertical_line(x,y, is_left_line):

    polarity = -1 if is_left_line else 1
    polyline = [(x, y)]


    # TODO: Convert to a non-while-true?
    while True:
        x += hexagon_half_width/2*polarity
        y += hexagon_half_height
        polyline.append((x, y))
        if y >= grid_height:
            # End on positive polarity
            #if (odd_row and polarity == -1) or (not odd_row and polarity == 1):
            #    break
            break
        if polarity == 1 and is_left_line or polarity == -1 and not is_left_line:
            y += trace_gap
            polyline.append((x, y))
        polarity *= -1
    return polyline


def hexagons_vertical_traces(doc):
    msp = doc.modelspace()
    layer = doc.layers.add(name="TopLayer")
    layer.transparency = TRANSPARENCY
    
    
    x, y = 0, 0
    odd_row = False
    while x < grid_width:
        polyline = []
        polyline.extend(hexagon_vertical_line(x, y, is_left_line=True))
        #msp.add_lwpolyline(polyline, dxfattribs={'layer': 'TopLayer', 'color': 1})  # Color 1 corresponds to red in ACI
        #polyline = []
        foo = hexagon_vertical_line(x + hexagon_side_length, y, is_left_line=False)
        foo.reverse()   
        polyline.extend(foo)
        #msp.add_lwpolyline(polyline, dxfattribs={'layer': 'TopLayer', 'color': 3})
      
            

        # optional: keep a closed lwpolyline for visibility
        #msp.add_lwpolyline(polyline, dxfattribs={'closed': True, 'layer': 'TopLayer'})
        # add a solid hatch to fill the polygon defined by `polyline`
        hatch = msp.add_hatch()
        hatch.set_solid_fill(1)               # choose ACI color index
        hatch.paths.add_polyline_path(polyline, is_closed=True)
        hatch.dxf.layer = 'TopLayer'
        
        polarity = 1 if odd_row else -1
        x += 3 * hexagon_half_width / 2 + trace_gap
        y += (hexagon_half_height + trace_gap/2)*polarity


        odd_row = not odd_row




def example_doc():
    doc = ezdxf.new()
    msp = doc.modelspace()
    x0, y0, x1, y1 = 0, 0, 10, 10
    start = (x0, y0)
    end = (x0 + 1, y0)
    for color in range(1, 6):
        msp.add_lwpolyline(
            [start, (x0, y1), (x1, y1), (x1, y0), end], dxfattribs={"color": color}
        )
        x0 += 1
        x1 -= 1
        y0 += 1
        y1 -= 1
        start = end
        end = (x0 + 1, y0)
    hexagons_vertical_traces(doc)
    hexagons_horizontal_traces(doc)
    return doc


def export(doc):
    msp = doc.modelspace()
    # 1. create the render context
    context = RenderContext(doc)
    # 2. create the backend
    backend = svg.SVGBackend()
    # 3. create the frontend
    frontend = Frontend(context, backend)
    # 4. draw the modelspace
    frontend.draw_layout(msp)
    # 5. create an A4 page layout, not required for all backends
    page = layout.Page(210, 297, layout.Units.mm, margins=layout.Margins.all(20))
    # 6. get the SVG rendering as string - this step is backend dependent
    svg_string = backend.get_string(page)
    with open("output.svg", "wt", encoding="utf8") as fp:
        fp.write(svg_string)


if __name__ == "__main__":
    export(example_doc())