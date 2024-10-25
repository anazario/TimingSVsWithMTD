#ifndef TimingWithSVs_TimingSVsWithMTD_TimingUtility_h
#define TimingWithSVs_TimingSVsWithMTD_TimingUtility_h

 struct TrackTofPidInfo {
    float tmtd;
    float tmtderror;
    float pathlength;

    float betaerror;

    float dt;
    float dterror;
    float dtchi2;

    float dt_best;
    float dterror_best;
    float dtchi2_best;

    float gammasq_pi;
    float beta_pi;
    float dt_pi;

    float gammasq_k;
    float beta_k;
    float dt_k;

    float gammasq_p;
    float beta_p;
    float dt_p;

    float prob_pi;
    float prob_k;
    float prob_p;
  };

inline const TrackTofPidInfo computeTrackTofPidInfo(float magp2,
						    float length,
						    float t_mtd,
						    float t_mtderr,
						    float t_vtx,
						    float t_vtx_err,
						    bool addPIDError = true) {

  constexpr float m_pi = 0.13957018f;
  constexpr float m_pi_inv2 = 1.0f / m_pi / m_pi;
  constexpr float m_k = 0.493677f;
  constexpr float m_k_inv2 = 1.0f / m_k / m_k;
  constexpr float m_p = 0.9382720813f;
  constexpr float m_p_inv2 = 1.0f / m_p / m_p;
  
  TrackTofPidInfo tofpid;
  
  tofpid.tmtd = t_mtd;
  tofpid.tmtderror = t_mtderr;
  tofpid.pathlength = length;
  /*
  auto deltat = [&](const float mass_inv2, const float betatmp) {
      float res(1.f);
      switch (choice) {
        case TofCalc::kCost:
          res = tofpid.pathlength / betatmp * c_inv;
          break;
        case TofCalc::kSegm:
          res = trs.computeTof(mass_inv2);
          break;
        case TofCalc::kMixd:
          res = trs.computeTof(mass_inv2) + tofpid.pathlength / betatmp * c_inv;
          break;
      }
      return res;
  };
  */

  auto deltat = [&](const float mass_inv2, const float betatmp) { return tofpid.pathlength / betatmp * c_inv;};
  
  tofpid.gammasq_pi = 1.f + magp2 * m_pi_inv2;
  tofpid.beta_pi = std::sqrt(1.f - 1.f / tofpid.gammasq_pi);
  tofpid.dt_pi = deltat(m_pi_inv2, tofpid.beta_pi);
  
  tofpid.gammasq_k = 1.f + magp2 * m_k_inv2;
  tofpid.beta_k = std::sqrt(1.f - 1.f / tofpid.gammasq_k);
  tofpid.dt_k = deltat(m_k_inv2, tofpid.beta_k);
  
  tofpid.gammasq_p = 1.f + magp2 * m_p_inv2;
  tofpid.beta_p = std::sqrt(1.f - 1.f / tofpid.gammasq_p);
  tofpid.dt_p = deltat(m_p_inv2, tofpid.beta_p);
  
  tofpid.dt = tofpid.tmtd - tofpid.dt_pi - t_vtx;  //assume by default the pi hypothesis
  tofpid.dterror = sqrt(tofpid.tmtderror * tofpid.tmtderror + t_vtx_err * t_vtx_err);
  tofpid.betaerror = 0.f;
  if (addPIDError) {
    tofpid.dterror = sqrt(tofpid.dterror * tofpid.dterror + (tofpid.dt_p - tofpid.dt_pi) * (tofpid.dt_p - tofpid.dt_pi));
    tofpid.betaerror = tofpid.beta_p - tofpid.beta_pi;
  }
  
  tofpid.dtchi2 = (tofpid.dt * tofpid.dt) / (tofpid.dterror * tofpid.dterror);
  
  tofpid.dt_best = tofpid.dt;
  tofpid.dterror_best = tofpid.dterror;
  tofpid.dtchi2_best = tofpid.dtchi2;
  
  tofpid.prob_pi = -1.f;
  tofpid.prob_k = -1.f;
  tofpid.prob_p = -1.f;

  if (!addPIDError) {
    //*TODO* deal with heavier nucleons and/or BSM case here?
    float chi2_pi = tofpid.dtchi2;
    float chi2_k =
      (tofpid.tmtd - tofpid.dt_k - t_vtx) * (tofpid.tmtd - tofpid.dt_k - t_vtx) / (tofpid.dterror * tofpid.dterror);
    float chi2_p =
      (tofpid.tmtd - tofpid.dt_p - t_vtx) * (tofpid.tmtd - tofpid.dt_p - t_vtx) / (tofpid.dterror * tofpid.dterror);
    
    float rawprob_pi = exp(-0.5f * chi2_pi);
    float rawprob_k = exp(-0.5f * chi2_k);
    float rawprob_p = exp(-0.5f * chi2_p);
    float normprob = 1.f / (rawprob_pi + rawprob_k + rawprob_p);
    
    tofpid.prob_pi = rawprob_pi * normprob;
    tofpid.prob_k = rawprob_k * normprob;
    tofpid.prob_p = rawprob_p * normprob;
    
    float prob_heavy = 1.f - tofpid.prob_pi;
    constexpr float heavy_threshold = 0.75f;
    
    if (prob_heavy > heavy_threshold) {
      if (chi2_k < chi2_p) {
	tofpid.dt_best = (tofpid.tmtd - tofpid.dt_k - t_vtx);
	tofpid.dtchi2_best = chi2_k;
      } else {
	tofpid.dt_best = (tofpid.tmtd - tofpid.dt_p - t_vtx);
	tofpid.dtchi2_best = chi2_p;
      }
    }
  }
  return tofpid;
}

#endif
