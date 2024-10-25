#ifndef TimingWithSVs_TimingSVsWithMTD_MTDIdInfo_h
#define TimingWithSVs_TimingSVsWithMTD_MTDIdInfo_h

#include "DataFormats/DetId/interface/DetId.h"
#include "TimingWithSVs/TimingSVsWithMTD/interface/MTDHitMatchingInfo.h"

class MTDIdInfo {

public:
  
  MTDIdInfo() = default;
  
  MTDIdInfo(const std::set<MTDHitMatchingInfo> &btlHitInfo, const std::set<MTDHitMatchingInfo> &etlHitInfo)
    : hasBTLHit_(btlHitInfo.size() > 0), hasETLHit_(etlHitInfo.size() > 0){

    hitInfo_.insert(btlHitInfo.begin(), btlHitInfo.end());
    hitInfo_.insert(etlHitInfo.begin(), etlHitInfo.end());
    /*
    std::cout << "\nThis track:" << std::endl;
    for (std::set<MTDHitMatchingInfo>::const_iterator it = hitInfo_.begin(); it != hitInfo_.end(); ++it) {

      std::cout << "time chi2: " << it->timeChi2 << ", estimated chi2: " << it->estChi2 << std::endl;
    }
    */
    for(const auto &info : btlHitInfo)
      crossedBTLIds_.push_back(info.id);

    for(const auto &info : etlHitInfo)
      crossedETLIds_.push_back(info.id);

    if(hasBTLHit_ && hasETLHit_)
      bestHit_ = *btlHitInfo.begin() < *etlHitInfo.begin() ? *btlHitInfo.begin() : *etlHitInfo.begin();
    else if (hasBTLHit_) bestHit_ = *btlHitInfo.begin();
    else if (hasETLHit_) bestHit_ = *etlHitInfo.begin();

    //std::cout << "best hit: time chi2: " << bestHit_.timeChi2 << ", estimated chi2: " << bestHit_.estChi2 << std::endl;
    //if(crossedETLIds_.size() == 2) std::cout << "Found a match with two ETL hits" << std::endl;
    
  }

  bool foundHit() const { return hasBTLHit_ || hasETLHit_; }
  bool foundHitBTL() const { return hasBTLHit_; }
  bool foundHitETL() const { return hasETLHit_; }

  double bestTime() const { return foundHit()? bestHit_.time : -999.; }
  double bestTimeError() const { return foundHit()? bestHit_.timeChi2 : -999.; }

  // This is the pathlength between the outermost state and the mtd hit.
  // For the full pathlength you need to propagate the track from the starting vertex position
  // to the outermost state and add it to this.
  double bestHitPathLength() const { return foundHit()? bestHit_.pathLengthTsosToMTD : -999.;}
  
  std::vector<DetId> crossedBTLIds() const {return crossedBTLIds_;}
  std::vector<DetId> crossedETLIds() const {return crossedETLIds_;}

  GlobalPoint tsosPosition() const {return bestHit_.tsosPosition;}
  
  virtual ~MTDIdInfo() = default;

 private:

  bool hasBTLHit_, hasETLHit_;
  MTDHitMatchingInfo bestHit_;
  std::set<MTDHitMatchingInfo> hitInfo_;
  std::vector<DetId> crossedBTLIds_;
  std::vector<DetId> crossedETLIds_;
};

#endif
