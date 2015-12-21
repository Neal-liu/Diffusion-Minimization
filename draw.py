from graph_tool.all import *
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

social_graph = open('synthetic.edge','r');
g = Graph()
count = 0
for line in social_graph:
    if(count == 0):
        count+=1
    else:
        print line
        e = g.add_edge(line[0], line[2])

'''
e = g.add_edge(1, 6)
e = g.add_edge(2, 6)
e = g.add_edge(3, 6)
e = g.add_edge(4, 6)
e = g.add_edge(5, 6)
'''
graph_draw(g, vertex_text=g.vertex_index, vertex_font_size=22, output="graph.png")
img = mpimg.imread('graph.png')
plt.imshow(img)
plt.show()

#graph_draw(g, edge_text=label, edge_font_size=40, edge_text_distance=20, edge_marker_size=40, output="output.png")

