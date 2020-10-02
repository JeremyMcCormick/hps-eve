#include "DetectorGeometry.h"

// HPS
#include "EventDisplay.h"
#include "FileCache.h"

// C++ standard library
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

// ROOT
#include "TEveElement.h"
#include "TEveGeoNode.h"
#include "TEveGeoShape.h"
#include "TEveTrans.h"
#include "TEveEventManager.h"
#include "TEveScene.h"

#ifdef HAVE_LIBXML2

#include <libxml/xmlreader.h>

void extractGdmlFile(const char* lcddName, const char* gdmlName) {

    xmlDocPtr doc = xmlParseFile(lcddName);
    if (doc == NULL) {
        throw std::runtime_error("Failed to open LCDD file.");
    }
    xmlNode *root = xmlDocGetRootElement(doc);

    xmlNode *cur = NULL;
    xmlNode *gdml = NULL;
    for (cur = root->children; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(cur->name, BAD_CAST "gdml")) {
                gdml = cur;
                break;
            }
        }
    }

    xmlDocPtr gdmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(gdmlDoc, gdml);

    FILE* gdmlFile = fopen(gdmlName, "wb");

    if (gdmlFile) {
        xmlDocDump(gdmlFile, gdmlDoc);
    } else {
        throw std::runtime_error("Failed to create new GDML file.");
    }

    fclose(gdmlFile);

    if (doc) {
        xmlFreeDoc(doc);
    }

    xmlCleanupParser();
}


#endif

namespace hps {

    DetectorGeometry::DetectorGeometry(EventDisplay* app, FileCache* cache) :
            Logger("DetectorGeometry"),
            geo_(nullptr),
            eve_(app->getEveManager()),
            fileCache_(cache) {
        setLogLevel(app->getLogLevel());
    }

    DetectorGeometry::~DetectorGeometry() {
    }

    TEveElementList* DetectorGeometry::createGeoElements(TGeoManager* geo,
                                                         const char* name,
                                                         const char* path,
                                                         const char* patt,
                                                         Char_t transparency) {
        auto elements = new TEveElementList(name);
        geo->cd(path);
        auto ndau = geo->GetCurrentNode()->GetNdaughters();
        for (int i=0; i<ndau; i++) {
            geo->CdDown(i);
            TGeoNode* node = geo->GetCurrentNode();
            if (std::string(node->GetName()).find(patt) != std::string::npos) {
                auto nodeName = node->GetName();
                TGeoVolume* vol = geo->GetCurrentVolume();
                TEveGeoShape* shape = new TEveGeoShape(node->GetName(), vol->GetMaterial()->GetName());
                shape->SetShape((TGeoShape*) vol->GetShape()->Clone());
                shape->SetMainColor(vol->GetLineColor());
                shape->SetFillColor(vol->GetFillColor());
                //shape->SetMainTransparency(vol->GetTransparency());
                shape->SetMainTransparency(transparency);
                shape->RefMainTrans().SetFrom(*geo->GetCurrentMatrix());
                elements->AddElement(shape);
            }
            geo->CdUp();
        }
        return elements;
    }

    void DetectorGeometry::addTracker(Char_t transparency) {
        log("Adding tracker...", INFO);
        auto tracker = new TEveElementList("SVT");
        std::string basePath("/world_volume_1/tracking_volume_0/base_volume_0");
        geo_->cd(basePath.c_str());
        auto base = geo_->GetCurrentNode();
        int n = base->GetNdaughters();
        for (int i=0; i<n; i++) {
            auto module = base->GetDaughter(i);
            std::string dauName(module->GetName());
            if (dauName.find("module_L") != std::string::npos) {
                for (int j=0; j<module->GetNdaughters(); j++) {
                    auto moduleDau = module->GetDaughter(j);
                    auto moduleDauName = std::string(moduleDau->GetName());
                    if (moduleDauName.find("sensor") != std::string::npos) {
                        std::stringstream ss;
                        ss << basePath << "/" << module->GetName() << "/" << moduleDauName;
                        geo_->cd(ss.str().c_str());
                        TGeoNode* node = geo_->GetCurrentNode();
                        TGeoVolume* volume = geo_->GetCurrentVolume();
                        std::string sensorName(node->GetName());
                        sensorName.replace(sensorName.find("_volume_0"), sizeof("_volume_0") - 1, "");
                        TEveGeoShape* shape = new TEveGeoShape(sensorName.c_str(), volume->GetMaterial()->GetName());
                        shape->SetShape((TGeoShape*) volume->GetShape()->Clone());
                        shape->SetMainColor(volume->GetLineColor());
                        shape->SetFillColor(volume->GetFillColor());
                        shape->SetMainTransparency(transparency);
                        shape->RefMainTrans().SetFrom(*geo_->GetCurrentMatrix());
                        tracker->AddElement(shape);
                        log(FINE) << "Added SVT volume: " << volume->GetName() << std::endl;
                    }
                }
            }
        }
        eve_->AddGlobalElement(tracker);
        log("Done adding tracker!", INFO);
    }

    void DetectorGeometry::addEcal(Char_t transparency) {
        log("Adding ECAL...", INFO);
        auto cal = createGeoElements(geo_,
                                     "ECAL",
                                     "/world_volume_1",
                                     "crystal_volume",
                                     transparency);
        cal->SetDrawOption("w");
        eve_->AddGlobalElement(cal);
        log("Done adding ECAL!", INFO);
    }

    void DetectorGeometry::addHodoscope(Char_t transparency) {
        log("Adding Hodoscope...", INFO);
        auto hodo = createGeoElements(geo_,
                                     "Hodoscope",
                                     "/world_volume_1/tracking_volume_0",
                                     "hodo_vol_L",
                                     transparency);
        //hodo->SetDrawOption("w");
        eve_->AddGlobalElement(hodo);
        log("Done adding Hodoscope!", INFO);
    }

    TEveElement* DetectorGeometry::toEveElement(TGeoManager* geo, TGeoNode* node) {
        TGeoVolume* vol = geo->GetCurrentVolume();
        TEveGeoShape* shape = new TEveGeoShape(node->GetName(), vol->GetMaterial()->GetName());
        shape->SetShape((TGeoShape*) vol->GetShape()->Clone());
        shape->SetMainColor(vol->GetLineColor());
        shape->SetFillColor(vol->GetFillColor());
        shape->SetMainTransparency(vol->GetTransparency());
        shape->RefMainTrans().SetFrom(*geo->GetCurrentMatrix());
        return shape;
    }

    TGeoManager* DetectorGeometry::getGeoManager() {
        return geo_;
    }

    // Include guards are just here for the compilation.
    // We will never get to this method if curl and libxml2 were not enabled.
    void DetectorGeometry::loadDetector(const std::string& detName) {

        log("Loading detector: " + detName, INFO);

        std::string lcddName = detName + ".lcdd";
        std::string gdmlName = detName + ".gdml";

        if (!fileCache_->isCached(lcddName)) {
            std::string detUrl = BASE_DETECTOR_URL + "/" + detName + "/" + detName + std::string(".lcdd");
            fileCache_->cache(detUrl.c_str(), lcddName.c_str());
            if (!fileCache_->isCached(lcddName)) {
                log("Failed to cache LCD file.", ERROR);
                throw new std::runtime_error("Failed to cache LCD file.");
            }
        }

        log("Extracting GDML file from: " + lcddName, INFO);
        if (!fileCache_->isCached(gdmlName)) {
#ifdef HAVE_LIBXML2
            extractGdmlFile(fileCache_->getCachedPath(lcddName).c_str(),
                            fileCache_->getCachedPath(gdmlName).c_str());
#else
            throw std::runtime_error("libxml2 was not enabled!");
#endif
            log("Done extracting GDML file!", INFO);
        } else {
            log("GDML file was already in cache: " + fileCache_->getCachedPath(gdmlName));
        }

        loadDetectorFile(fileCache_->getCachedPath(gdmlName));

        log("Done loading detector!", INFO);
    }

    void DetectorGeometry::loadDetectorFile(const std::string& gdmlName) {
        log("Loading GDML file: " + gdmlName);
        geo_ = TGeoManager::Import(gdmlName.c_str());
        log("Building detector...", FINE);
        buildDetector();
        log("Done building detector!", FINE);

    }

    void DetectorGeometry::buildDetector() {
        addTracker();
        addEcal();
        addHodoscope();
    }

    bool DetectorGeometry::isInitialized() {
        return geo_ != nullptr;
    }
}
