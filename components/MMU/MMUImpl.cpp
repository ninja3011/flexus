﻿// DO-NOT-REMOVE begin-copyright-block 
//
// Redistributions of any form whatsoever must retain and/or include the
// following acknowledgment, notices and disclaimer:
//
// This product includes software developed by Carnegie Mellon University.
//
// Copyright 2012 by Mohammad Alisafaee, Eric Chung, Michael Ferdman, Brian 
// Gold, Jangwoo Kim, Pejman Lotfi-Kamran, Onur Kocberber, Djordje Jevdjic, 
// Jared Smolens, Stephen Somogyi, Evangelos Vlachos, Stavros Volos, Jason 
// Zebchuk, Babak Falsafi, Nikos Hardavellas and Tom Wenisch for the SimFlex 
// Project, Computer Architecture Lab at Carnegie Mellon, Carnegie Mellon University.
//
// For more information, see the SimFlex project website at:
//   http://www.ece.cmu.edu/~simflex
//
// You may not use the name "Carnegie Mellon University" or derivations
// thereof to endorse or promote products derived from this software.
//
// If you modify the software you must place a notice on or within any
// modified version provided or made available to any third party stating
// that you have modified the software.  The notice shall include at least
// your name, address, phone number, email address and the date and purpose
// of the modification.
//
// THE SOFTWARE IS PROVIDED "AS-IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER
// EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO ANY WARRANTY
// THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS OR BE ERROR-FREE AND ANY
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// TITLE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
// BE LIABLE FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO DIRECT, INDIRECT,
// SPECIAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF, RESULTING FROM, OR IN
// ANY WAY CONNECTED WITH THIS SOFTWARE (WHETHER OR NOT BASED UPON WARRANTY,
// CONTRACT, TORT OR OTHERWISE).
//
// DO-NOT-REMOVE end-copyright-block

// Changelog:
//  - June'18: msutherl - basic TLB definition, no real timing info

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <components/MMU/MMU.hpp>

#include <core/performance/profile.hpp>
#include <core/qemu/configuration_api.hpp>

#include <components/CommonQEMU/Translation.hpp>
#include "MMUUtil.hpp"
#include "pageWalk.hpp"

#include <map>
#include <queue>
#include <vector>

#define DBG_DefineCategories MMU
#define DBG_SetDefaultOps AddCat(MMU)
#include DBG_Control()
#define DBG_DefineCategories TLBMissTracking
#include DBG_Control()

#define FLEXUS_BEGIN_COMPONENT MMU
#include FLEXUS_BEGIN_COMPONENT_IMPLEMENTATION()



namespace nMMU {

using namespace Flexus;
using namespace Core;
using namespace SharedTypes;

class FLEXUS_COMPONENT(MMU) {
  FLEXUS_COMPONENT_IMPL(MMU);

private:


  std::unique_ptr<PageWalk> thePageWalker;
  typedef std::map<VirtualMemoryAddress, PhysicalMemoryAddress> TLBentry;
  TLBentry theInstrTLB;
  TLBentry theDataTLB;

  std::queue<boost::intrusive_ptr<Translation>> theLookUpEntries;
  std::queue<boost::intrusive_ptr<Translation>> thePageWalkEntries;

  std::shared_ptr<Flexus::Qemu::Processor> theCPU;
  std::shared_ptr<mmu_t> theMMU;

  bool theMMUInitialized;

public:
  FLEXUS_COMPONENT_CONSTRUCTOR(MMU)
    : base ( FLEXUS_PASS_CONSTRUCTOR_ARGS )
  {}


  bool isQuiesced() const {
      return false;
  }

  void saveState(std::string const & aDirName) {
    // TODO
  }

  void loadState(std::string const & aDirName) {
//    TODO
  }

  // Initialization
  void initialize() {
      theCPU = std::make_shared<Flexus::Qemu::Processor>(Flexus::Qemu::Processor::getProcessor((int)flexusIndex()));
      thePageWalker.reset(new PageWalk(flexusIndex()));
      thePageWalker->setMMU(theMMU);
  }

  void finalize() {}

  //MMUDrive
  //----------
  void drive(interface::MMUDrive const &) {
    DBG_(VVerb, Comp(*this) ( << "MMUDrive" ) ) ;
    busCycle();
    thePageWalker->cycle();
    processMemoryRequests();

  }


  void busCycle() {

      while (!theLookUpEntries.empty()){

          boost::intrusive_ptr<Translation> item = theLookUpEntries.front();
          theLookUpEntries.pop();

          if (item->isInstr()) {
              if (theInstrTLB.find(item->theVaddr) != theInstrTLB.end()) {
                    // item exists so mark hit
                item->setHit();
                FLEXUS_CHANNEL(iTranslationReply) << item;
              } else {
                  // mark miss
                  item->setMiss();
                  thePageWalkEntries.push(item);
                  thePageWalker->push_back(item);
              }
          } else if (item->isData()) {
              if (theDataTLB.find(item->theVaddr) != theDataTLB.end()) {
                    // item exists so mark hit
                  item->setHit();
                  FLEXUS_CHANNEL(dTranslationReply) << item;
              } else {
                  // mark miss
                  item->setMiss();
                  thePageWalkEntries.push(std::move(item));
                  thePageWalker->push_back(item);
              }
          } else {
              assert(false);
          }
      }

      while (!thePageWalkEntries.empty()){
          boost::intrusive_ptr<Translation> item = thePageWalkEntries.front();

          if (item->isDone()){
              thePageWalkEntries.pop();

              if (item->isInstr()){
                theInstrTLB[item->theVaddr] = item->thePaddr;
                FLEXUS_CHANNEL(iTranslationReply) << item;
              } else if (item->isData()){
                  theDataTLB[item->theVaddr] = item->thePaddr;
                  FLEXUS_CHANNEL(dTranslationReply) << item;
              } else {
                  DBG_Assert(false);
              }
          }
          else {
              break;
          }
      }
  }


  void processMemoryRequests(){
      while (thePageWalker->hasMemoryRequest()){
          TranslationPtr tmp = thePageWalker->popMemoryRequest();
          FLEXUS_CHANNEL(MemoryRequestOut) << tmp;
      }
  }


  void initMMU(uint8_t anIndex) {

    if (!theMMUInitialized){
        theMMU.reset(new mmu_t());
        theMMUInitialized = true;
    }
    theMMU->initRegsFromQEMUObject( getMMURegsFromQEMU(anIndex) );
    theMMU->setupAddressSpaceSizesAndGranules();

    DBG_(Tmp,( << "MMU object init'd, " << std::hex << theMMU << std::dec ));
  }

  // Msutherl: Fetch MMU's registers
  std::shared_ptr<mmu_regs_t> getMMURegsFromQEMU(uint8_t anIndex) {
      std::shared_ptr<mmu_regs_t> mmu_obj (new mmu_regs_t());

      mmu_obj->SCTLR[EL0] = Flexus::Qemu::Processor::getProcessor(anIndex)->readSCTLR(EL0);
      mmu_obj->SCTLR[EL1] = Flexus::Qemu::Processor::getProcessor(anIndex)->readSCTLR(EL1);
      mmu_obj->SCTLR[EL2] = Flexus::Qemu::Processor::getProcessor(anIndex)->readSCTLR(EL2);
      mmu_obj->SCTLR[EL3] = Flexus::Qemu::Processor::getProcessor(anIndex)->readSCTLR(EL3);

      mmu_obj->TCR[EL0] =   Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TCR, EL0);
      mmu_obj->TCR[EL1] =   Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TCR, EL1);
      mmu_obj->TCR[EL2] =   Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TCR, EL2);
      mmu_obj->TCR[EL3] =   Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TCR, EL3);
      mmu_obj->TTBR1[EL0] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR1,EL0);
      mmu_obj->TTBR0[EL1] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR0,EL1);
      mmu_obj->TTBR1[EL1] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR1,EL1);
      mmu_obj->TTBR0[EL2] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR0,EL2);
      mmu_obj->TTBR1[EL2] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR1,EL2);
      mmu_obj->TTBR0[EL3] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR0,EL3);
      mmu_obj->TTBR1[EL3] = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_TTBR1,EL3);

      mmu_obj->ID_AA64MMFR0_EL1 = Qemu::API::QEMU_read_register(* Flexus::Qemu::Processor::getProcessor(anIndex),Qemu::API::kMMU_ID_AA64MMFR0_EL1, 0);

      return mmu_obj;
  }

  bool IsTranslationEnabledAtEL(uint8_t & anEL) {
      return true; // theCore->IsTranslationEnabledAtEL(anEL);
  }

  bool available( interface::ResyncIn const &,
                  index_t anIndex) {
    return true;
  }
  void push( interface::ResyncIn const &,
             index_t           anIndex,
             bool & aResync ) {
      initMMU(anIndex);
  }

  bool available( interface::iRequestIn const &,
                  index_t anIndex) {
    return true;
  }
  void push( interface::iRequestIn const &,
             index_t           anIndex,
             TranslationPtr& aTranslate ) {
      aTranslate->toggleReady();
      theLookUpEntries.push( aTranslate );
  }

  bool available( interface::dRequestIn const &,
                  index_t anIndex) {
    return true;
  }
  void push( interface::dRequestIn const &,
             index_t           anIndex,
             TranslationPtr& aTranslate ) {
      theLookUpEntries.push( aTranslate );
  }


  void sendTLBresponse(TranslationPtr aTranslation) {
    if (aTranslation->isInstr())
        FLEXUS_CHANNEL(iTranslationReply) << aTranslation;
    else
        FLEXUS_CHANNEL(dTranslationReply) << aTranslation;
  }
};

} //End Namespace nMMU

FLEXUS_COMPONENT_INSTANTIATOR( MMU , nMMU );
FLEXUS_PORT_ARRAY_WIDTH( MMU , dRequestIn )           {
  return (cfg.Cores);
}
FLEXUS_PORT_ARRAY_WIDTH( MMU , iRequestIn )    {
  return (cfg.Cores);
}
FLEXUS_PORT_ARRAY_WIDTH( MMU , ResyncIn )           {
  return (cfg.Cores);
}
FLEXUS_PORT_ARRAY_WIDTH( MMU , iTranslationReply )    {
  return (cfg.Cores);
}
FLEXUS_PORT_ARRAY_WIDTH( MMU , dTranslationReply )    {
  return (cfg.Cores);
}

FLEXUS_PORT_ARRAY_WIDTH( MMU , MemoryRequestOut )    {
  return (cfg.Cores);
}

#include FLEXUS_END_COMPONENT_IMPLEMENTATION()
#define FLEXUS_END_COMPONENT MMU

#define DBG_Reset
#include DBG_Control()
