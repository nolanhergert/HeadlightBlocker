"""Can you create a python file that uses ezdxf to lay out the top and bottom ITO layers for this sunglasses project? The outline of the glasses shape should be roughly a rayban wayfarers shape, but let's focus on only one eye. (the left eye) There is a hexagonal pattern, like discussed above, with 50um gap and .95mm pixel width, and the dimensions of the main grid should be roughly 35mm wide and 15mm tall. The grid center's x and y coordinates should match a typical person wearing this style of sunglasses (taking inter-pupil distance and nose resting position into account)


The rest of the space in the wayfarer-style lens should continue the pattern row/column pattern up until the edge. Leave a few row and column traces to be able to turn these large regions off (to allow the entire surface of the sunglasses to be dimmable still, not just the rectangular-shaped area near the eyes). This will also theoretically allow for at-the-factory dynamic lens cuts in the future from the same ITO pattern.



The bottom ITO surface should be the rows, and the traces come from a chip-on-glass lcd  driver (ST7065C, 2310x1830um) mounted in the top right.



The top ITO surface should be the columns, which derive from a second ST7065C chip, and whose traces should, in general, wrap along the right side of the row traces and join up with the hexagonal grid columns along the bottom of the sunglasses.



All specific values  should be adjustable / variables.

Output the result as a dxf and a png/pdf/similar for preview.



Feel free to ask any clarifying questions if you are unsure."""

'''


I'll create a Python script to generate the ITO layers for your LCD sunglasses. Let me ask a few clarifying questions first:

Hexagon orientation: Should the hexagons be "flat-topped" (two horizontal edges) or "pointy-topped" (two vertical edges)? Flat-topped is typically better for horizontal/vertical routing.
Row/column assignment: For the hexagonal grid, how should rows and columns be defined?

Option A: Rows follow the horizontal bands of hexagons, columns are the diagonal lines
Option B: Some other mapping scheme?


Trace routing:

Should the row traces route to the top-right along the top edge of the lens?
Should column traces route along the right edge then bottom edge?
What trace width do you want (e.g., 100 μm)?


Wayfarer lens dimensions:

Total lens width (horizontal): ~55mm?
Total lens height (vertical): ~45mm?
Should I include the characteristic angled top edge?


Inter-pupillary distance (IPD): Standard adult IPD is ~63mm. Should the grid center be positioned assuming:

The lens frame is centered on the pupil?
Or offset considering bridge width?


Extended grid regions: When you say "continue the pattern... to allow the entire surface to be dimmable", should I:

Create separate addressable regions (e.g., "top peripheral", "bottom peripheral") with a few rows/columns each?
Or extend the main grid all the way to edges?



Let me know your preferences and I'll build the script!
'''



'''
1. I think flat-topped
2 See attached, although it is pointy-topped. I think this explains what you are asking.
3. Row traces for the hexagonal grid shouldn't go on the top edge, but rather the right edge. Of course, there are two extra row traces that should go on the top and bottom to "cover" the column traces. Correct on routing column traces on right and then bottom edge. Trace width of 50um for now, with 50um spacing.
4. Yes, that width and height sound good. I'll have specific dimensions later. I'm not sure what you mean by angled top edge, but you can include it.
5. However a wayfarer would typically fall on a typical face. I think it is not centered necessarily. Maybe allow adjusting the center of the grid?
6. Extend main row and column traces out to the edges, but "cover" them with an extra wide column (for the left side) and extra tall rows (top and bottom). Hope this makes sense.
'''

#!/usr/bin/env python3
"""
Generate ITO layer layouts for hexagonal LCD sunglasses display.
Creates separate DXF files for top (columns) and bottom (rows) ITO layers.
"""

import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as MplPolygon
from matplotlib.patches import Rectangle as MplRectangle

# ============================================================================
# CONFIGURATION PARAMETERS
# ============================================================================

# Hexagon parameters
PIXEL_WIDTH = 0.950  # mm (width across flats for flat-topped hexagon)
GAP_WIDTH = 0.050    # mm (gap between pixels)
HEX_ORIENTATION = "flat"  # flat-topped hexagons

# Main grid dimensions
GRID_WIDTH = 35.0   # mm (approximate width of main viewing area)
GRID_HEIGHT = 15.0  # mm (approximate height of main viewing area)

# Grid center position (relative to lens center)
GRID_CENTER_X = 0.0  # mm (0 = centered on lens)
GRID_CENTER_Y = 2.0  # mm (positive = up, accounting for nose bridge position)

# Wayfarer lens outline
LENS_WIDTH = 55.0   # mm
LENS_HEIGHT = 45.0  # mm
LENS_CENTER_X = 0.0 # mm
LENS_CENTER_Y = 0.0 # mm

# LCD Driver chip (ST7065C)
CHIP_WIDTH = 2.310   # mm
CHIP_HEIGHT = 1.830  # mm
CHIP_POSITION_X = LENS_WIDTH / 2 - 3.0  # mm (top right area)
CHIP_POSITION_Y = LENS_HEIGHT / 2 - 3.0  # mm

# Trace parameters
TRACE_WIDTH = 0.050  # mm
TRACE_SPACING = 0.050  # mm

# Output files
OUTPUT_DXF_BOTTOM = "sunglasses_bottom_rows.dxf"
OUTPUT_DXF_TOP = "sunglasses_top_columns.dxf"
OUTPUT_PREVIEW = "sunglasses_preview.png"

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

class SimpleDXF:
    """Simple DXF file writer for basic geometry."""
    
    def __init__(self, filename):
        self.filename = filename
        self.entities = []
        self.handle_counter = 100
        
    def add_polyline(self, points, closed=False, layer='0', color=7, width=0):
        """Add a polyline to the DXF."""
        entity = {
            'type': 'LWPOLYLINE',
            'points': points,
            'closed': closed,
            'layer': layer,
            'color': color,
            'width': width
        }
        self.entities.append(entity)
        
    def add_line(self, start, end, layer='0', color=7):
        """Add a line to the DXF."""
        entity = {
            'type': 'LINE',
            'start': start,
            'end': end,
            'layer': layer,
            'color': color
        }
        self.entities.append(entity)
    
    def write(self):
        """Write the DXF file."""
        with open(self.filename, 'w') as f:
            # DXF Header
            f.write("0\nSECTION\n2\nHEADER\n")
            f.write("9\n$ACADVER\n1\nAC1015\n")  # AutoCAD 2000
            f.write("0\nENDSEC\n")
            
            # Tables section
            f.write("0\nSECTION\n2\nTABLES\n")
            
            # Layer table
            f.write("0\nTABLE\n2\nLAYER\n70\n4\n")
            
            layers = [
                ('0', 7),
                ('HEXAGONS', 4),  # Cyan
                ('TRACES', 1),    # Red
                ('CHIP', 2),      # Yellow
                ('OUTLINE', 7),   # White
            ]
            
            for layer_name, color in layers:
                f.write(f"0\nLAYER\n2\n{layer_name}\n70\n0\n62\n{color}\n6\nCONTINUOUS\n")
            
            f.write("0\nENDTAB\n")
            f.write("0\nENDSEC\n")
            
            # Entities section
            f.write("0\nSECTION\n2\nENTITIES\n")
            
            for entity in self.entities:
                if entity['type'] == 'LWPOLYLINE':
                    self._write_polyline(f, entity)
                elif entity['type'] == 'LINE':
                    self._write_line(f, entity)
            
            f.write("0\nENDSEC\n")
            f.write("0\nEOF\n")
    
    def _write_polyline(self, f, entity):
        """Write a LWPOLYLINE entity."""
        f.write("0\nLWPOLYLINE\n")
        f.write(f"8\n{entity['layer']}\n")
        f.write(f"62\n{entity['color']}\n")
        f.write(f"90\n{len(entity['points'])}\n")
        f.write(f"70\n{1 if entity['closed'] else 0}\n")
        
        if entity['width'] > 0:
            f.write(f"43\n{entity['width']}\n")
        
        for x, y in entity['points']:
            f.write(f"10\n{x}\n20\n{y}\n")
    
    def _write_line(self, f, entity):
        """Write a LINE entity."""
        f.write("0\nLINE\n")
        f.write(f"8\n{entity['layer']}\n")
        f.write(f"62\n{entity['color']}\n")
        f.write(f"10\n{entity['start'][0]}\n20\n{entity['start'][1]}\n")
        f.write(f"11\n{entity['end'][0]}\n21\n{entity['end'][1]}\n")


def create_wayfarer_outline():
    """
    Create a Wayfarer-style lens outline.
    Returns list of (x, y) points defining the outline.
    """
    points = []
    
    # Simplified Wayfarer shape (left lens)
    # Characteristic features: angled top edge, rounded bottom
    
    w = LENS_WIDTH / 2
    h = LENS_HEIGHT / 2
    cx = LENS_CENTER_X
    cy = LENS_CENTER_Y
    
    # Start from top-left, go clockwise
    # Top edge (angled)
    points.append((cx - w, cy + h * 0.9))
    points.append((cx - w * 0.3, cy + h))
    points.append((cx + w * 0.7, cy + h))
    
    # Right edge (slightly curved)
    points.append((cx + w, cy + h * 0.6))
    points.append((cx + w, cy + h * 0.2))
    points.append((cx + w, cy - h * 0.2))
    
    # Bottom edge (rounded)
    points.append((cx + w * 0.8, cy - h))
    points.append((cx + w * 0.4, cy - h * 0.95))
    points.append((cx, cy - h * 0.95))
    points.append((cx - w * 0.4, cy - h * 0.95))
    points.append((cx - w * 0.8, cy - h))
    
    # Left edge (curved, bridge side)
    points.append((cx - w, cy - h * 0.4))
    points.append((cx - w, cy))
    points.append((cx - w, cy + h * 0.4))
    
    return points


def point_in_polygon(point, polygon):
    """Check if point is inside polygon using ray casting algorithm."""
    x, y = point
    n = len(polygon)
    inside = False
    
    p1x, p1y = polygon[0]
    for i in range(1, n + 1):
        p2x, p2y = polygon[i % n]
        if y > min(p1y, p2y):
            if y <= max(p1y, p2y):
                if x <= max(p1x, p2x):
                    if p1y != p2y:
                        xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                    if p1x == p2x or x <= xinters:
                        inside = not inside
        p1x, p1y = p2x, p2y
    
    return inside


def create_flat_hexagon(center_x, center_y, width):
    """
    Create a flat-topped hexagon.
    Width is the distance across flats (horizontal distance).
    Returns list of 6 (x, y) vertices.
    """
    # For flat-topped hexagon:
    # width = sqrt(3) * side_length
    # height = 2 * side_length
    
    side_length = width / math.sqrt(3)
    
    # Flat-topped: vertices at 0°, 60°, 120°, 180°, 240°, 300° from center
    vertices = []
    for i in range(6):
        angle = math.radians(60 * i)
        x = center_x + side_length * math.cos(angle)
        y = center_y + side_length * math.sin(angle)
        vertices.append((x, y))
    
    return vertices


def get_hexagon_metrics(width):
    """Calculate hexagon dimensions."""
    side_length = width / math.sqrt(3)
    height = 2 * side_length
    
    # For flat-topped "odd-r" layout:
    # - Vertical spacing between row centers
    vert_spacing = height * 0.75  # 3/4 of height
    
    # - Horizontal spacing between column centers
    horiz_spacing = width
    
    return {
        'side_length': side_length,
        'height': height,
        'vert_spacing': vert_spacing,
        'horiz_spacing': horiz_spacing
    }


def calculate_hex_grid(grid_width, grid_height, hex_width, center_x, center_y):
    """
    Calculate hexagon positions for the main grid.
    Uses "odd-r" horizontal layout (odd rows offset to the right).
    Returns dict with hexagon data.
    """
    metrics = get_hexagon_metrics(hex_width)
    
    # Calculate how many rows and columns we need
    n_rows = int(grid_height / metrics['vert_spacing']) + 2
    n_cols = int(grid_width / metrics['horiz_spacing']) + 2
    
    # Make sure n_rows and n_cols are adequate
    n_rows = max(n_rows, 10)
    n_cols = max(n_cols, 20)
    
    hexagons = []
    
    # Start position (top-left of grid)
    start_x = center_x - (n_cols // 2) * metrics['horiz_spacing']
    start_y = center_y + (n_rows // 2) * metrics['vert_spacing']
    
    for row in range(n_rows):
        for col in range(n_cols):
            # "odd-r" layout: odd rows are offset to the right by half the horizontal spacing
            offset_x = metrics['horiz_spacing'] / 2 if row % 2 == 1 else 0
            
            hex_x = start_x + col * metrics['horiz_spacing'] + offset_x
            hex_y = start_y - row * metrics['vert_spacing']
            
            hexagons.append({
                'row': row,
                'col': col,
                'center': (hex_x, hex_y),
                'vertices': create_flat_hexagon(hex_x, hex_y, hex_width)
            })
    
    return {
        'hexagons': hexagons,
        'n_rows': n_rows,
        'n_cols': n_cols,
        'metrics': metrics
    }


def filter_hexagons_in_lens(hexagons, lens_outline):
    """Filter hexagons to only those inside the lens outline."""
    filtered = []
    for hex_data in hexagons:
        # Check if center is inside lens outline
        if point_in_polygon(hex_data['center'], lens_outline):
            filtered.append(hex_data)
    
    return filtered


def draw_hexagon_dxf(dxf, vertices, layer_name, color=4):
    """Draw a hexagon as a closed polyline in DXF."""
    dxf.add_polyline(vertices, closed=True, layer=layer_name, color=color)


def draw_trace_dxf(dxf, start, end, width, layer_name, color=1):
    """Draw a trace as a wide polyline in DXF."""
    dxf.add_polyline([start, end], closed=False, layer=layer_name, 
                    color=color, width=width)


def draw_chip_outline_dxf(dxf, center_x, center_y, width, height, layer_name, color=2):
    """Draw chip outline as rectangle."""
    x1 = center_x - width / 2
    y1 = center_y - height / 2
    x2 = center_x + width / 2
    y2 = center_y + height / 2
    
    points = [(x1, y1), (x2, y1), (x2, y2), (x1, y2)]
    dxf.add_polyline(points, closed=True, layer=layer_name, color=color)


# ============================================================================
# MAIN GENERATION FUNCTIONS
# ============================================================================

def generate_bottom_layer_rows(hexagons, lens_outline, grid_data):
    """
    Generate bottom ITO layer (rows).
    Each row is a horizontal band of hexagons connected together.
    """
    dxf = SimpleDXF(OUTPUT_DXF_BOTTOM)
    
    # Draw lens outline (white, color 7)
    dxf.add_polyline(lens_outline, closed=True, layer='OUTLINE', color=7)
    
    # Draw chip (yellow, color 2)
    draw_chip_outline_dxf(dxf, CHIP_POSITION_X, CHIP_POSITION_Y, 
                          CHIP_WIDTH, CHIP_HEIGHT, 'CHIP', color=2)
    
    # Group hexagons by row
    rows = {}
    for hex_data in hexagons:
        row_idx = hex_data['row']
        if row_idx not in rows:
            rows[row_idx] = []
        rows[row_idx].append(hex_data)
    
    # Draw hexagons and create row traces
    chip_x = CHIP_POSITION_X - CHIP_WIDTH / 2
    chip_y_start = CHIP_POSITION_Y - CHIP_HEIGHT / 2
    
    trace_y_offset = 0
    
    for row_idx in sorted(rows.keys()):
        row_hexagons = rows[row_idx]
        
        # Draw all hexagons in this row (cyan, color 4)
        for hex_data in row_hexagons:
            draw_hexagon_dxf(dxf, hex_data['vertices'], 'HEXAGONS', color=4)
        
        # Find rightmost hexagon in this row
        rightmost = max(row_hexagons, key=lambda h: h['center'][0])
        right_x = rightmost['center'][0] + get_hexagon_metrics(PIXEL_WIDTH)['horiz_spacing'] / 2
        right_y = rightmost['center'][1]
        
        # Route trace from rightmost hexagon to right edge, then up to chip
        edge_x = LENS_CENTER_X + LENS_WIDTH / 2 - 1.0  # 1mm from edge
        
        # Horizontal trace to edge (red, color 1)
        draw_trace_dxf(dxf, (right_x, right_y), (edge_x, right_y), 
                      TRACE_WIDTH, 'TRACES', color=1)
        
        # Vertical trace along edge to chip connection point
        chip_connect_y = chip_y_start + trace_y_offset
        draw_trace_dxf(dxf, (edge_x, right_y), (edge_x, chip_connect_y), 
                      TRACE_WIDTH, 'TRACES', color=1)
        
        # Horizontal trace to chip
        draw_trace_dxf(dxf, (edge_x, chip_connect_y), (chip_x, chip_connect_y), 
                      TRACE_WIDTH, 'TRACES', color=1)
        
        trace_y_offset += TRACE_WIDTH + TRACE_SPACING
    
    # Add extra rows at top and bottom to cover column traces
    # Top row
    top_y = LENS_CENTER_Y + LENS_HEIGHT / 2 - 2.0
    draw_trace_dxf(dxf, 
                  (LENS_CENTER_X - LENS_WIDTH / 2, top_y),
                  (LENS_CENTER_X + LENS_WIDTH / 2, top_y), 
                  2.0, 'TRACES', color=1)  # Extra wide
    
    # Bottom row
    bottom_y = LENS_CENTER_Y - LENS_HEIGHT / 2 + 2.0
    draw_trace_dxf(dxf, 
                  (LENS_CENTER_X - LENS_WIDTH / 2, bottom_y),
                  (LENS_CENTER_X + LENS_WIDTH / 2, bottom_y), 
                  2.0, 'TRACES', color=1)  # Extra wide
    
    dxf.write()
    print(f"Bottom layer (rows) saved to: {OUTPUT_DXF_BOTTOM}")
    
    return dxf


def generate_top_layer_columns(hexagons, lens_outline, grid_data):
    """
    Generate top ITO layer (columns).
    Each column is a vertical series of hexagons connected together.
    """
    dxf = SimpleDXF(OUTPUT_DXF_TOP)
    
    # Draw lens outline (white, color 7)
    dxf.add_polyline(lens_outline, closed=True, layer='OUTLINE', color=7)
    
    # Draw chip (second chip for columns, yellow, color 2)
    chip2_y = CHIP_POSITION_Y - CHIP_HEIGHT - 2.0  # Below the row chip
    draw_chip_outline_dxf(dxf, CHIP_POSITION_X, chip2_y, 
                          CHIP_WIDTH, CHIP_HEIGHT, 'CHIP', color=2)
    
    # Group hexagons by column
    columns = {}
    for hex_data in hexagons:
        col_idx = hex_data['col']
        if col_idx not in columns:
            columns[col_idx] = []
        columns[col_idx].append(hex_data)
    
    # Draw hexagons and create column traces
    chip_x_start = CHIP_POSITION_X - CHIP_WIDTH / 2
    chip_y = chip2_y - CHIP_HEIGHT / 2
    
    trace_x_offset = 0
    
    for col_idx in sorted(columns.keys()):
        col_hexagons = columns[col_idx]
        
        # Draw all hexagons in this column (cyan, color 4)
        for hex_data in col_hexagons:
            draw_hexagon_dxf(dxf, hex_data['vertices'], 'HEXAGONS', color=4)
        
        # Find bottommost hexagon in this column
        bottommost = min(col_hexagons, key=lambda h: h['center'][1])
        bottom_x = bottommost['center'][0]
        bottom_y = bottommost['center'][1] - get_hexagon_metrics(PIXEL_WIDTH)['vert_spacing'] / 2
        
        # Route trace from bottommost hexagon to bottom edge, then right, then up to chip
        edge_y = LENS_CENTER_Y - LENS_HEIGHT / 2 + 1.0  # 1mm from bottom edge
        edge_x = LENS_CENTER_X + LENS_WIDTH / 2 - 1.0   # Right edge
        
        # Vertical trace to bottom edge (green, color 3)
        draw_trace_dxf(dxf, (bottom_x, bottom_y), (bottom_x, edge_y), 
                      TRACE_WIDTH, 'TRACES', color=3)
        
        # Horizontal trace along bottom edge to right side
        draw_trace_dxf(dxf, (bottom_x, edge_y), (edge_x, edge_y), 
                      TRACE_WIDTH, 'TRACES', color=3)
        
        # Vertical trace along right edge to chip connection point
        chip_connect_x = chip_x_start + trace_x_offset
        draw_trace_dxf(dxf, (edge_x, edge_y), (edge_x, chip_y), 
                      TRACE_WIDTH, 'TRACES', color=3)
        
        # Horizontal trace to chip
        draw_trace_dxf(dxf, (edge_x, chip_y), (chip_connect_x, chip_y), 
                      TRACE_WIDTH, 'TRACES', color=3)
        
        trace_x_offset += TRACE_WIDTH + TRACE_SPACING
    
    # Add extra wide column on left side to cover row traces
    left_x = LENS_CENTER_X - LENS_WIDTH / 2 + 2.0
    draw_trace_dxf(dxf,
                  (left_x, LENS_CENTER_Y - LENS_HEIGHT / 2),
                  (left_x, LENS_CENTER_Y + LENS_HEIGHT / 2), 
                  2.0, 'TRACES', color=3)  # Extra wide
    
    dxf.write()
    print(f"Top layer (columns) saved to: {OUTPUT_DXF_TOP}")
    
    return dxf


def create_preview_image(hexagons, lens_outline, grid_data):
    """Create a preview image showing both layers."""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 8))
    
    # Bottom layer (rows)
    ax1.set_title('Bottom Layer - Rows (Red)', fontsize=14, fontweight='bold')
    ax1.set_aspect('equal')
    ax1.grid(True, alpha=0.3)
    
    # Draw lens outline
    outline_poly = MplPolygon(lens_outline, fill=False, edgecolor='white', linewidth=2)
    ax1.add_patch(outline_poly)
    
    # Draw hexagons colored by row
    rows = {}
    for hex_data in hexagons:
        row_idx = hex_data['row']
        if row_idx not in rows:
            rows[row_idx] = []
        rows[row_idx].append(hex_data)
    
    colors_list = plt.cm.tab20(np.linspace(0, 1, len(rows)))
    for i, row_idx in enumerate(sorted(rows.keys())):
        for hex_data in rows[row_idx]:
            hex_poly = MplPolygon(hex_data['vertices'], 
                                 facecolor=colors_list[i], 
                                 edgecolor='black', 
                                 linewidth=0.5,
                                 alpha=0.7)
            ax1.add_patch(hex_poly)
    
    ax1.set_xlim(LENS_CENTER_X - LENS_WIDTH/2 - 5, LENS_CENTER_X + LENS_WIDTH/2 + 5)
    ax1.set_ylim(LENS_CENTER_Y - LENS_HEIGHT/2 - 5, LENS_CENTER_Y + LENS_HEIGHT/2 + 5)
    ax1.set_xlabel('X (mm)')
    ax1.set_ylabel('Y (mm)')
    
    # Top layer (columns)
    ax2.set_title('Top Layer - Columns (Green)', fontsize=14, fontweight='bold')
    ax2.set_aspect('equal')
    ax2.grid(True, alpha=0.3)
    
    # Draw lens outline
    outline_poly2 = MplPolygon(lens_outline, fill=False, edgecolor='white', linewidth=2)
    ax2.add_patch(outline_poly2)
    
    # Draw hexagons colored by column
    columns = {}
    for hex_data in hexagons:
        col_idx = hex_data['col']
        if col_idx not in columns:
            columns[col_idx] = []
        columns[col_idx].append(hex_data)
    
    colors_list2 = plt.cm.tab20b(np.linspace(0, 1, len(columns)))
    for i, col_idx in enumerate(sorted(columns.keys())):
        for hex_data in columns[col_idx]:
            hex_poly = MplPolygon(hex_data['vertices'], 
                                 facecolor=colors_list2[i % len(colors_list2)], 
                                 edgecolor='black', 
                                 linewidth=0.5,
                                 alpha=0.7)
            ax2.add_patch(hex_poly)
    
    ax2.set_xlim(LENS_CENTER_X - LENS_WIDTH/2 - 5, LENS_CENTER_X + LENS_WIDTH/2 + 5)
    ax2.set_ylim(LENS_CENTER_Y - LENS_HEIGHT/2 - 5, LENS_CENTER_Y + LENS_HEIGHT/2 + 5)
    ax2.set_xlabel('X (mm)')
    ax2.set_ylabel('Y (mm)')
    
    plt.tight_layout()
    plt.savefig(OUTPUT_PREVIEW, dpi=300, facecolor='black')
    print(f"Preview image saved to: {OUTPUT_PREVIEW}")
    plt.close()


# ============================================================================
# MAIN EXECUTION
# ============================================================================

def main():
    print("Generating ITO layer layouts for hexagonal LCD sunglasses...")
    print(f"Pixel width: {PIXEL_WIDTH} mm")
    print(f"Gap width: {GAP_WIDTH} mm")
    print(f"Grid dimensions: {GRID_WIDTH} x {GRID_HEIGHT} mm")
    print(f"Lens dimensions: {LENS_WIDTH} x {LENS_HEIGHT} mm")
    print()
    
    # Create lens outline
    lens_outline = create_wayfarer_outline()
    
    # Calculate hexagonal grid
    grid_data = calculate_hex_grid(
        GRID_WIDTH, 
        GRID_HEIGHT, 
        PIXEL_WIDTH,
        GRID_CENTER_X,
        GRID_CENTER_Y
    )
    
    print(f"Generated {len(grid_data['hexagons'])} hexagons")
    print(f"Grid size: {grid_data['n_rows']} rows x {grid_data['n_cols']} columns")
    
    # Filter hexagons to those inside lens
    hexagons_in_lens = filter_hexagons_in_lens(grid_data['hexagons'], lens_outline)
    print(f"Hexagons inside lens: {len(hexagons_in_lens)}")
    print()
    
    # Generate bottom layer (rows)
    print("Generating bottom layer (rows)...")
    generate_bottom_layer_rows(hexagons_in_lens, lens_outline, grid_data)
    
    # Generate top layer (columns)
    print("Generating top layer (columns)...")
    generate_top_layer_columns(hexagons_in_lens, lens_outline, grid_data)
    
    # Create preview
    print("Creating preview image...")
    create_preview_image(hexagons_in_lens, lens_outline, grid_data)
    
    print()
    print("Done! Files generated:")
    print(f"  - {OUTPUT_DXF_BOTTOM}")
    print(f"  - {OUTPUT_DXF_TOP}")
    print(f"  - {OUTPUT_PREVIEW}")


if __name__ == "__main__":
    main()