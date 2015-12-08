from graph_tool.all import *

g = Graph()
label = g.new_edge_property("string")

e = g.add_edge(1, 6)
label[e] = 1.0
e = g.add_edge(2, 6)
label[e] = 2.3
e = g.add_edge(3, 6)
label[e] = 4.678
e = g.add_edge(4, 6)
#label[e] = 1
e = g.add_edge(5, 6)
#label[e] = 1

graph_draw(g, edge_text=label, edge_font_size=40, output="output.png")
#graph_draw(g, edge_text=label, edge_font_size=40, edge_text_distance=20, edge_marker_size=40, output="output.png")

