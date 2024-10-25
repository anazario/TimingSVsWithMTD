## Instructions

To set up the environment, clone the repository, and build the project, follow these commands:

```bash
cmsrel CMSSW_14_0_0
cd CMSSW_14_0_0/src
mkdir TimingWithSVs
cd TimingWithSVs
git clone git@github.com:anazario/TimingSVsWithMTD.git
cd TimingSVsWithMTD
scram b -j9
