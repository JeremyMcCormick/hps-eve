import ROOT
from ROOT import TEveGeoTopNode, TEveManager

mgr = TEveManager.Create()
geo = mgr.GetGeometry("./HPS-PhysicsRun2019-v2-4pt5.gdml")
top = geo.GetTopNode()
world = TEveGeoTopNode(geo, top)
mgr.AddGlobalElement(world)
mgr.FullRedraw3D(True)
view = mgr.GetDefaultGLViewer()
view.DoDraw()

input('Press any key to continue...')
