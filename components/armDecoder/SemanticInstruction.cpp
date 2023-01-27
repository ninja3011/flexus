//  DO-NOT-REMOVE begin-copyright-block
// QFlex consists of several software components that are governed by various
// licensing terms, in addition to software that was developed internally.
// Anyone interested in using QFlex needs to fully understand and abide by the
// licenses governing all the software components.
//
// ### Software developed externally (not by the QFlex group)
//
//     * [NS-3] (https://www.gnu.org/copyleft/gpl.html)
//     * [QEMU] (http://wiki.qemu.org/License)
//     * [SimFlex] (http://parsa.epfl.ch/simflex/)
//     * [GNU PTH] (https://www.gnu.org/software/pth/)
//
// ### Software developed internally (by the QFlex group)
// **QFlex License**
//
// QFlex
// Copyright (c) 2020, Parallel Systems Architecture Lab, EPFL
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of the Parallel Systems Architecture Laboratory, EPFL,
//       nor the names of its contributors may be used to endorse or promote
//       products derived from this software without specific prior written
//       permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE PARALLEL SYSTEMS ARCHITECTURE LABORATORY,
// EPFL BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  DO-NOT-REMOVE end-copyright-block

#include <list>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <components/uArchARM/uArchInterfaces.hpp>
#include <components/uFetch/uFetchTypes.hpp>
#include <core/boost_extensions/intrusive_ptr.hpp>
#include <core/debug/debug.hpp>
#include <core/performance/profile.hpp>
#include <core/qemu/mai_api.hpp>
#include <core/stats.hpp>
#include <core/target.hpp>
#include <core/types.hpp>

#include "Effects.hpp"
#include "OperandMap.hpp"
#include "SemanticActions.hpp"
#include "SemanticInstruction.hpp"
#include "Validations.hpp"

#define DBG_DeclareCategories armDecoder
#define DBG_SetDefaultOps AddCat(armDecoder)
#include DBG_Control

namespace narmDecoder {

uint32_t theInsnCount;
Flexus::Stat::StatCounter theICBs("sys-ICBs");
Flexus::Stat::StatMax thePeakInsns("sys-PeakSemanticInsns");

std::set<SemanticInstruction *> theGlobalLiveInsns;
int64_t theLastPrintCount = 0;

void SemanticInstruction::constructorTrackLiveInsns() {
  if (theLastPrintCount >= 10000) {
    DBG_(Dev, (<< "Live Insn Count: " << theInsnCount));
    theLastPrintCount = 0;
    if (theInsnCount > 10000) {
      DBG_(Dev, (<< "Identifying oldest live instruction."));
      DBG_Assert(!theGlobalLiveInsns.empty());
      SemanticInstruction *oldest = *theGlobalLiveInsns.begin();
      std::set<SemanticInstruction *>::iterator iter = theGlobalLiveInsns.begin();
      while (iter != theGlobalLiveInsns.end()) {
        if ((*iter)->sequenceNo() < oldest->sequenceNo()) {
          oldest = *iter;
        }
        ++iter;
      }
      DBG_(Dev, (<< "Oldest live insn: " << *oldest));
      DBG_(Dev, (<< "   Ref count: " << oldest->theRefCount));
      DBG_(Dev, (<< "Oldest live insn internals: " << std::internal << *oldest));
    }
  }
  ++theLastPrintCount;
  theGlobalLiveInsns.insert(this);
}

void SemanticInstruction::constructorInitValidations() {
  ++theInsnCount;
  thePeakInsns << theInsnCount;
  for (int32_t i = 0; i < 4; ++i) {
    theRetirementDepends[i] = true;
  }
  addPrevalidation(validatePC(this, true));
}

SemanticInstruction::SemanticInstruction(VirtualMemoryAddress aPC, Opcode anOpcode,
                                         boost::intrusive_ptr<BPredState> aBPState, uint32_t aCPU,
                                         int64_t aSequenceNo, eInstructionClass aClass,
                                         eInstructionCode aCode)
    : armInstruction(aPC, anOpcode, aBPState, aCPU, aSequenceNo, aClass, aCode),
      theOverrideSimics(false), thePrevalidationsPassed(false), theRetireDepCount(0),
      theIsMicroOp(false), theRetirementTarget(*this), theCanRetireCounter(0) {
  constructorInitValidations();
#ifdef TRACK_INSNS
  constructorTrackLiveInsns();
#endif // TRACK_INSNS
}

SemanticInstruction::SemanticInstruction(VirtualMemoryAddress aPC, Opcode anOpcode,
                                         boost::intrusive_ptr<BPredState> aBPState, uint32_t aCPU,
                                         int64_t aSequenceNo)
    : armInstruction(aPC, anOpcode, aBPState, aCPU, aSequenceNo), theOverrideSimics(false),
      thePrevalidationsPassed(false), theRetireDepCount(0), theIsMicroOp(false),
      theRetirementTarget(*this), theCanRetireCounter(0) {
  constructorInitValidations();
#ifdef TRACK_INSNS
  constructorTrackLiveInsns();
#endif // TRACK_INSNS
}

SemanticInstruction::~SemanticInstruction() {
  --theInsnCount;

#ifdef TRACK_INSNS
  theGlobalLiveInsns.erase(this);
#endif // TRACK_INSNS
}

nuArchARM::InstructionDependance
SemanticInstruction::makeInstructionDependance(InternalDependance const &aDependance) {
  DBG_Assert(reinterpret_cast<long>(aDependance.theTarget) != 0x1);
  nuArchARM::InstructionDependance ret_val;
  ret_val.instruction = boost::intrusive_ptr<nuArchARM::Instruction>(this);
  ret_val.satisfy =
      boost::bind(&DependanceTarget::invokeSatisfy, aDependance.theTarget, aDependance.theArg);
  ret_val.squash =
      boost::bind(&DependanceTarget::invokeSquash, aDependance.theTarget, aDependance.theArg);
  return ret_val;
}

void SemanticInstruction::setMayRetire(int32_t aBit, bool aFlag) {
  SEMANTICS_TRACE;
  DBG_Assert(aBit < theRetireDepCount,
             (<< "setMayRetire setting a bit that is out of range: " << aBit << "\n"
              << std::internal << *this));
  bool may_retire = mayRetire();
  SEMANTICS_DBG("aBit = " << aBit << ", aFlag = " << aFlag << ", " << *this);
  theRetirementDepends[aBit] = aFlag;
  if (mayRetire() && !may_retire) {
    DBG_(Iface, (<< identify() << " may retire"));
  } else if (!mayRetire() && may_retire) {
    DBG_(Iface, (<< identify() << " may not retire"));
  }
}

bool SemanticInstruction::mayRetire() const {
  FLEXUS_PROFILE();
  if (isPageFault())
    return true;
  bool ok = theRetirementDepends[0] && theRetirementDepends[1] && theRetirementDepends[2] &&
            theRetirementDepends[3];
  for (std::list<std::function<bool()>>::const_iterator iter = theRetirementConstraints.begin(),
                                                        end = theRetirementConstraints.end();
       ok && (iter != end); ++iter) {
    ok &= (*iter)();
  }

  ok &= (theCanRetireCounter == 0);

  return ok; // May retire if no dependence bit is clear
}

void SemanticInstruction::setCanRetireCounter(const uint32_t numCycles) {
  theCanRetireCounter = numCycles;
}

void SemanticInstruction::decrementCanRetireCounter() {
  if (theCanRetireCounter > 0) {
    --theCanRetireCounter;
  }
}

InternalDependance SemanticInstruction::retirementDependance() {
  DBG_Assert(theRetireDepCount < 5);
  theRetirementDepends[theRetireDepCount] = false;
  return InternalDependance(&theRetirementTarget, theRetireDepCount++);
}

bool SemanticInstruction::preValidate() {
  FLEXUS_PROFILE();
  bool ok = true;
  while (ok && !thePreValidations.empty()) {
    ok &= thePreValidations.front()();
    thePreValidations.pop_front();
  }
  thePrevalidationsPassed = ok;
  return ok;
}

bool SemanticInstruction::advancesSimics() const {
  return (!theAnnulled && !theIsMicroOp);
}

void SemanticInstruction::overrideSimics() {
  theOverrideSimics = true;
}

bool SemanticInstruction::postValidate() {
  FLEXUS_PROFILE();
  if (theRaisedException != willRaise()) {
    DBG_(VVerb, (<< *this << " PostValidation failed: Exception mismatch flexus=" << std::hex
                 << willRaise() << " simics=" << theRaisedException << std::dec));
    return false;
  }
  //  if ( Flexus::Qemu::Processor::getProcessor(theCPU)->getPC()  != theNPC &&
  //  ! theRaisedException ) {//NOOSHIN, getNPC instead of getPC
  //    DBG_( VVerb, ( << *this << " PostValidation failed: NPC mismatch
  //    flexus=" << theNPC << " simics=" <<
  //    Flexus::Qemu::Processor::getProcessor(theCPU)->getPC() ) ); return
  //    false;
  //  }

  if (thePrevalidationsPassed && theOverrideSimics && !theOverrideFns.empty() &&
      !theRaisedException && !theAnnulled) {
    DBG_(VVerb, (<< *this << " overrideSimics flag set and override conditions passed."));
    while (!theOverrideFns.empty()) {
      theOverrideFns.front()();
      theOverrideFns.pop_front();
    }
  }
  bool ok = true;
  while (ok && !thePostValidations.empty()) {
    ok &= thePostValidations.front()();
    thePostValidations.pop_front();
  }
  return ok;
}

void SemanticInstruction::addPrevalidation(std::function<bool()> aValidation) {
  thePreValidations.push_back(aValidation);
}

void SemanticInstruction::addPostvalidation(std::function<bool()> aValidation) {
  thePostValidations.push_back(aValidation);
}

void SemanticInstruction::addOverride(std::function<void()> anOverride) {
  theOverrideFns.push_back(anOverride);
}

void SemanticInstruction::addRetirementConstraint(std::function<bool()> aConstraint) {
  theRetirementConstraints.push_back(aConstraint);
}

void SemanticInstruction::annul() {
  FLEXUS_PROFILE();
  if (!theAnnulled) {
    armInstruction::annul();
    theAnnulmentEffects.invoke(*this);
  }
}

void SemanticInstruction::reinstate() {
  FLEXUS_PROFILE();
  if (theAnnulled) {
    armInstruction::reinstate();
    theReinstatementEffects.invoke(*this);
  }
}

void SemanticInstruction::doDispatchEffects() {
  DISPATCH_DBG("START DISPATCHING ACTIONS");
  FLEXUS_PROFILE();
  armInstruction::doDispatchEffects();
  while (!theDispatchActions.empty()) {
    core()->create(theDispatchActions.front());
    theDispatchActions.pop_front();
  }
  DISPATCH_DBG("FINISH DISPATCHING ACTIONS");

  DISPATCH_DBG("START DISPATCHING EFFECTS");
  theDispatchEffects.invoke(*this);
  DISPATCH_DBG("FINISH DISPATCHING EFFECTS");
};

void SemanticInstruction::doRetirementEffects() {
  FLEXUS_PROFILE();
  theRetirementEffects.invoke(*this);
  theRetired = true;
  // Clear predecessor to avoid leaking instructions
  thePredecessor = 0;
};

void SemanticInstruction::checkTraps() {
  FLEXUS_PROFILE();
  theCheckTrapEffects.invoke(*this);
};

void SemanticInstruction::doCommitEffects() {
  FLEXUS_PROFILE();
  theCommitEffects.invoke(*this);
  // Clear predecessor to avoid leaking instructions
  thePredecessor = 0;
};

void SemanticInstruction::pageFault() {
  thePageFault = true;
}

bool SemanticInstruction::isPageFault() const {
  return thePageFault;
}

void SemanticInstruction::squash() {
  FLEXUS_PROFILE();
  DBG_(VVerb, (<< *this << " squashed"));
  theSquashed = true;
  theSquashEffects.invoke(*this);
  // Clear predecessor to avoid leaking instructions
  thePredecessor = 0;
};

void SemanticInstruction::addDispatchEffect(Effect *anEffect) {
  theDispatchEffects.append(anEffect);
}

void SemanticInstruction::addDispatchAction(simple_action const &anAction) {
  theDispatchActions.push_back(anAction.action);
}

void SemanticInstruction::addRetirementEffect(Effect *anEffect) {
  theRetirementEffects.append(anEffect);
}

void SemanticInstruction::addCheckTrapEffect(Effect *anEffect) {
  theCheckTrapEffects.append(anEffect);
}

void SemanticInstruction::addCommitEffect(Effect *anEffect) {
  theCommitEffects.append(anEffect);
}

void SemanticInstruction::addSquashEffect(Effect *anEffect) {
  theSquashEffects.append(anEffect);
}

void SemanticInstruction::addAnnulmentEffect(Effect *anEffect) {
  theAnnulmentEffects.append(anEffect);
}

void SemanticInstruction::addReinstatementEffect(Effect *anEffect) {
  theReinstatementEffects.append(anEffect);
}

void SemanticInstruction::describe(std::ostream &anOstream) const {
  bool internals = false;
  if (anOstream.flags() & std::ios_base::internal) {
    anOstream << std::left;
    internals = true;
  }
  armInstruction::describe(anOstream);
  if (theRetired) {
    anOstream << "\t{retired}";
  } else if (theSquashed) {
    anOstream << "\t{squashed}";
  } else if (mayRetire()) {
    anOstream << "\t{completed}";
  } else if (hasExecuted()) {
    anOstream << "\t{executed}";
  }
  if (theAnnulled) {
    anOstream << "\t{annulled}";
  }
  if (theIsMicroOp) {
    anOstream << "\t{micro-op}";
  }

  if (internals) {
    anOstream << std::endl;
    anOstream << "\tOperandMap:\n";
    theOperands.dump(anOstream);

    if (!theDispatchEffects.empty()) {
      anOstream << "\tDispatchEffects:\n";
      theDispatchEffects.describe(anOstream);
    }

    if (!theAnnulmentEffects.empty()) {
      anOstream << "\tAnnulmentEffects:\n";
      theAnnulmentEffects.describe(anOstream);
    }

    if (!theReinstatementEffects.empty()) {
      anOstream << "\tReinstatementEffects:\n";
      theReinstatementEffects.describe(anOstream);
    }

    if (!theSquashEffects.empty()) {
      anOstream << "\tSquashEffects:\n";
      theSquashEffects.describe(anOstream);
    }

    if (!theRetirementEffects.empty()) {
      anOstream << "\tRetirementEffects:\n";
      theRetirementEffects.describe(anOstream);
    }

    if (!theCommitEffects.empty()) {
      anOstream << "\tCommitEffects:\n";
      theCommitEffects.describe(anOstream);
    }
  }
}

eExceptionType SemanticInstruction::retryTranslation() {
  DBG_Assert(false);
  return eExceptionType(NULL);
  //  Flexus::SharedTypes::Translation xlat;
  //  xlat.theVaddr = pc();
  //  //xlat.theTL = core()->getTL();
  //  xlat.thePSTATE = core()->getPSTATE();
  //  xlat.theType = Flexus::SharedTypes::Translation::eFetch;
  //  Flexus::Qemu::Processor::getProcessor(theCPU)->translate(xlat);
  //  return kException_None/*xlat.theException*/;
}

PhysicalMemoryAddress SemanticInstruction::translate() {
  DBG_Assert(false);
  return PhysicalMemoryAddress(0);
  //  Flexus::SharedTypes::Translation xlat;
  //  xlat.theVaddr = pc();
  //  xlat.thePSTATE = core()->getPSTATE();
  //  xlat.theType = Flexus::SharedTypes::Translation::eFetch;
  //  Flexus::Qemu::Processor::getProcessor(theCPU)->translate(xlat);
  //  return xlat.thePaddr;
}

size_t SemanticInstruction::addNewComponent(UncountedComponent *aComponent) {
  return theICB.addNewComponent(aComponent);
}

} // namespace narmDecoder
