/**
 * \class L1TStage2uGT
 *
 * Description: DQM for L1 Micro Global Trigger.
 *
 * \author Mateusz Zarucki 2016
 * \author J. Berryhill, I. Mikulec
 * \author Vasile Mihai Ghete - HEPHY Vienna
 *
 */

#include "DQM/L1TMonitor/interface/L1TStage2uGT.h"

// Constructor
L1TStage2uGT::L1TStage2uGT(const edm::ParameterSet& params):
   l1tStage2uGtSource_(consumes<GlobalAlgBlkBxCollection>(params.getParameter<edm::InputTag>("l1tStage2uGtSource"))),
   verbose_(params.getUntrackedParameter<bool>("verbose", false))
{
   histFolder_ = params.getUntrackedParameter<std::string> ("HistFolder", "L1T2016/L1TStage2uGT");
}

// Destructor
L1TStage2uGT::~L1TStage2uGT() {
   // empty
}

void L1TStage2uGT::dqmBeginRun(edm::Run const& iRun, edm::EventSetup const& evtSetup) {
   // empty 
}

void L1TStage2uGT::beginLuminosityBlock(const edm::LuminosityBlock& iLumi, const edm::EventSetup& evtSetup) { 
   // empty
}

void L1TStage2uGT::bookHistograms(DQMStore::IBooker &ibooker, edm::Run const&, edm::EventSetup const& evtSetup) {
   
   // Book histograms
   const int numLS = 1000;
   const double numLS_d = static_cast<double>(numLS);
   const int numAlgs = 512; // FIXME: Take number of algorithms from EventSetup
   const double numAlgs_d = static_cast<double>(numAlgs)-0.5;

   ibooker.setCurrentFolder(histFolder_);
    
   algoBits_ = ibooker.book1D("algoBits", "uGT: Algorithm Trigger Bits", numAlgs, -0.5, numAlgs_d);
   algoBits_->setAxisTitle("Algorithm Trigger Bits", 1);
   
   algoBits_corr_ = ibooker.book2D("algoBits_corr","uGT: Algorithm Trigger Bit Correlation", numAlgs, -0.5, numAlgs_d, numAlgs, -0.5, numAlgs_d);
   algoBits_corr_->setAxisTitle("Algorithm Trigger Bits", 1);
   algoBits_corr_->setAxisTitle("Algorithm Trigger Bits", 2);
   
   algoBits_bx_global_ = ibooker.book2D("algoBits_bx_global", "uGT: Algorithm Trigger Bits vs. Global BX Number", 3600, -0.5, 3599.5, numAlgs, -0.5, numAlgs_d);
   algoBits_bx_global_->setAxisTitle("Global Bunch Crossing Number", 1); 
   algoBits_bx_global_->setAxisTitle("Algorithm Trigger Bits", 2);
   
   algoBits_bx_inEvt_ = ibooker.book2D("algoBits_bx_inEvt", "uGT: Algorithm Trigger Bits vs. BX Number in Event", 5, -2.5, 2.5, numAlgs, -0.5, numAlgs_d);
   algoBits_bx_inEvt_->setAxisTitle("Bunch Crossing Number in Event", 1);
   algoBits_bx_inEvt_->setAxisTitle("Algorithm Trigger Bits", 2);
   
   algoBits_lumi_ = ibooker.book2D("algoBits_lumi","uGT: Algorithm Trigger Bits vs. LS", numLS, 0., numLS_d, numAlgs, -0.5, numAlgs_d);
   algoBits_lumi_->setAxisTitle("Luminosity Segment", 1);
   algoBits_lumi_->setAxisTitle("Algorithm Trigger Bits", 2);
 
   prescaleFactorSet_ = ibooker.book2D("prescaleFactorSet", "uGT: Index of Prescale Factor Set vs. LS", numLS, 0., numLS_d, 25, 0., 25.);
   prescaleFactorSet_->setAxisTitle("Luminosity Segment", 1);
   prescaleFactorSet_->setAxisTitle("Prescale Factor Set Index", 2);
}

void L1TStage2uGT::analyze(const edm::Event& evt, const edm::EventSetup& evtSetup) {
   // FIXME: Remove duplicate definition of numAlgs 
   const int numAlgs = 512;
  
   if (verbose_) {
      edm::LogInfo("L1TStage2uGT") << "L1TStage2uGT DQM: Analyzing.." << std::endl;
   }
   
   // Get standard event parameters 
   int lumi = evt.luminosityBlock();
   int bx = evt.bunchCrossing();
      
   // Open uGT readout record
   edm::Handle<GlobalAlgBlkBxCollection> uGtAlgs;
   evt.getByToken(l1tStage2uGtSource_, uGtAlgs);
   
   if (!uGtAlgs.isValid()) {
      edm::LogInfo("L1TStage2uGT") << "Cannot find uGT readout record.";
      return;
   }
   
   // Get uGT algo bit statistics
   else {
      //algoBits_->Fill(-1.); // fill underflow to normalize // FIXME: needed? 
      for (int ibx=uGtAlgs->getFirstBX(); ibx <= uGtAlgs->getLastBX(); ++ibx) {
         for (auto itr = uGtAlgs->begin(ibx); itr != uGtAlgs->end(ibx); ++itr) { // FIXME: redundant loop over 1-dim vector?
            
            // Fills prescale factor set histogram
            prescaleFactorSet_->Fill(lumi, itr->getPreScColumn());
             
            // Fills algorithm bits histograms
            for(int algoBit = 0; algoBit < numAlgs; ++algoBit) {
               if(itr->getAlgoDecisionFinal(algoBit)) {
                  algoBits_->Fill(algoBit);
                  algoBits_lumi_->Fill(lumi, algoBit);
                  algoBits_bx_global_->Fill(bx, algoBit);
                  algoBits_bx_inEvt_->Fill(ibx, algoBit); // FIXME: or itr->getbxInEventNr()/getbxNr()?
                  
                  for(int algoBit2 = 0; algoBit2 < numAlgs; ++algoBit2) {
                     if(itr->getAlgoDecisionFinal(algoBit2)) {
                        algoBits_corr_->Fill(algoBit, algoBit2);
                     }
                  }
               }  
            }
         }
      }
   }
}

// End section
void L1TStage2uGT::endLuminosityBlock(const edm::LuminosityBlock& iLumi, const edm::EventSetup& evtSetup) {
   // empty
}
