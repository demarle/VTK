import vtk
import inspect

def parseClassTree(classtree, tree, names, parentId):
    """
    This function convertes a nested list generated by
    inspect.getclasstree() to a vtkTree.
    """
    
    childId = None
    for i in classtree:
        # element showing the parent
        if type(i) == type(()):
            child = None
            for j in i:
                if type(j) == type(()):
                    pass
                else:
                    child = j.__name__
            if tree.GetNumberOfNodes() == 0:
                childId = tree.AddRoot()
            else:
                childId = tree.AddChild(parentId)
            names.InsertValue(childId, child)
        # element showing the children
        elif type(i) == type([]):
            parseClassTree(i, tree, names, childId)

# Get a list of all wrapped vtk classes
list = []
for cls in dir(vtk):
    clsObj = vtk.__dict__[cls]
    if type(clsObj) == type(vtk.vtkObject):
        list.append(clsObj)

# The tree that will hold vtk class hierarchy
tree = vtk.vtkTree()
names = vtk.vtkStringArray()
names.SetName("name")

# Allocate 100 tuples to start with. I picked this number
# out of air.
names.SetNumberOfTuples(100)

# Use inspect to get the class tree
classtree = inspect.getclasstree(list, 1)

# Convert nested list to vtkTree
parseClassTree(classtree, tree, names, None)

tree.GetPointData().AddArray(names)

## Now iterate over the tree and print it
#iter = vtk.vtkTreeDFSIterator()
#iter.SetTree(tree)
#
#while iter.HasNext():
#    id = iter.Next()
#    mystr = tree.GetLevel(id) * "   "
#    print mystr + str(names.GetValue(id))

# Display the tree in the tree map viewer
viewer = vtk.vtkTreeMapViewer()
viewer.SetInput(tree);

win = vtk.vtkRenderWindow()
interact = vtk.vtkRenderWindowInteractor()
interact.SetRenderWindow(win)
viewer.SetRenderWindow(win)

win.GetInteractor().Initialize()
win.GetInteractor().Start();

