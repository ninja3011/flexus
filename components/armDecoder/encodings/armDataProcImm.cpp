// DO-NOT-REMOVE begin-copyright-block
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

#include "armDataProcImm.hpp"
#include "armUnallocated.hpp"
#include "armSharedFunctions.hpp"

namespace narmDecoder {
using namespace nuArchARM;


enum eMoveWideOp {
    kMoveWideOp_N,
    kMoveWideOp_Z,
    kMoveWideOp_K,
};

arminst ADR(armcode const & aFetchedOpcode, uint32_t aCPU, int64_t aSequenceNo)
{
    DECODER_TRACE;
    SemanticInstruction* inst(new SemanticInstruction(aFetchedOpcode.thePC,aFetchedOpcode.theOpcode,
                                                      aFetchedOpcode.theBPState, aCPU,aSequenceNo));


    inst->setClass(clsComputation, codeALU);

    uint32_t  rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    int64_t offset = sextract64(aFetchedOpcode.theOpcode, 5, 19);
    offset = (offset << 2) | extract32(aFetchedOpcode.theOpcode, 29, 2);
    bool op = extract32(aFetchedOpcode.theOpcode, 31, 1);
    bits base = aFetchedOpcode.thePC;

    if (op) {
        /* ADRP (page based) */
        base &= ~0xfff;
        offset <<= 12;
    }
    std::vector<std::list<InternalDependance> > rs_deps(2);

    addReadConstant(inst, 1, base, rs_deps[0]);
    addReadConstant(inst, 2, offset, rs_deps[1]);

    predicated_action exec = addExecute(inst, operation(kADD_), rs_deps);  

    addDestination(inst, rd, exec, true);

    return inst;
}

arminst EXTR(armcode const & aFetchedOpcode, uint32_t aCPU, int64_t aSequenceNo)
{
    DECODER_TRACE;
    bool sf = extract32(aFetchedOpcode.theOpcode, 31, 1);
    bool n = extract32(aFetchedOpcode.theOpcode, 22, 1);
    uint32_t rm = extract32(aFetchedOpcode.theOpcode, 16, 5);
    uint32_t imm = extract32(aFetchedOpcode.theOpcode, 10, 6);
    uint32_t rn = extract32(aFetchedOpcode.theOpcode, 5, 5);
    uint32_t rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    uint32_t op21 = extract32(aFetchedOpcode.theOpcode, 29, 2);
    bool op0 = extract32(aFetchedOpcode.theOpcode, 21, 1);
    bool bitsize = sf ? 64 : 32;

    if (sf != n || op21 || op0 || imm >= bitsize) {
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }

    SemanticInstruction* inst(new SemanticInstruction(aFetchedOpcode.thePC,aFetchedOpcode.theOpcode,
                                                      aFetchedOpcode.theBPState, aCPU,aSequenceNo));


    inst->setClass(clsComputation, codeALU);

    std::vector<std::list<InternalDependance> > rs_deps(3);
    addReadXRegister(inst, 1, rn, rs_deps[0], sf);
    addReadXRegister(inst, 2, rm, rs_deps[1], sf);

    addReadConstant(inst, 3, imm, rs_deps[2]);

    predicated_action exec = extractAction(inst, rs_deps, kOperand1, kOperand2, kOperand3, sf);

    addDestination(inst, rd, exec, sf);

    return inst;
}

arminst BFM(armcode const & aFetchedOpcode, uint32_t aCPU, int64_t aSequenceNo)
{

    DECODER_TRACE;

    uint32_t rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    uint32_t rn = extract32(aFetchedOpcode.theOpcode, 5, 5);
    uint32_t imms = extract32(aFetchedOpcode.theOpcode, 10, 6);
    uint32_t immr = extract32(aFetchedOpcode.theOpcode, 16, 6);
    bool n = extract32(aFetchedOpcode.theOpcode, 22, 1);
    uint32_t opc = extract32(aFetchedOpcode.theOpcode, 29, 2);
    bool sf = extract32(aFetchedOpcode.theOpcode, 31, 1);
    bool inzero, extend;

    if ((!sf && n) || (sf && !n) || (opc == 2)){//FIXME ReservedValue();
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }
    if (!sf && (n || ((immr & (1 << 5)) != 0) || ((imms & (1 << 5)) != 0))) {//FIXME ReservedValue();
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }


    switch (opc) {
    case 0: // SBFM
        inzero = true;
        extend = true;
        break;
    case 1: // BFM
        break;
    case 2: // SBFM
        inzero = true;
        break;
    default:
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
        break;
    }



    bits wmask, tmask;
    if (! decodeBitMasks(tmask, wmask, n, imms, immr, false, sf ? 64 : 32) ){
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }

    SemanticInstruction * inst( new SemanticInstruction(aFetchedOpcode.thePC, aFetchedOpcode.theOpcode, aFetchedOpcode.theBPState, aCPU, aSequenceNo) );

    inst->setClass(clsComputation, codeALU);

    std::vector<std::list<InternalDependance>> rs_deps(1);
    addReadXRegister(inst, 1, rn, rs_deps[0], sf);
    if (inzero){
        addReadConstant(inst, 2, 0, rs_deps[1]);
    } else {
        addReadXRegister(inst, 2, rd, rs_deps[1], sf);
    }


    predicated_action exec = bitFieldAction(inst, rs_deps, kOperand1, kOperand2, imms, immr, wmask, tmask, extend, sf);
    addDestination(inst, rd, exec, sf);

    return inst;
}

arminst MOVE(armcode const & aFetchedOpcode, uint32_t  aCPU, int64_t aSequenceNo)
{
    DECODER_TRACE;
    uint32_t rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    uint32_t imm = extract32(aFetchedOpcode.theOpcode, 5, 16);
    bool sf = extract32(aFetchedOpcode.theOpcode, 31, 1);
    uint32_t pos = extract32(aFetchedOpcode.theOpcode, 21, 2) << 4;
    uint32_t opc = extract32(aFetchedOpcode.theOpcode, 29, 2);

    if (!sf && (pos >= 32)) {
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }

    eMoveWideOp opcode;
    switch (opc) {
    case 0:
        opcode = kMoveWideOp_N;
        break;
    case 1:
        opcode = kMoveWideOp_Z;
        break;
    case 2:
        opcode = kMoveWideOp_K;
        break;
    default:
        return unallocated_encoding(aFetchedOpcode, aCPU,aSequenceNo);
        break;
    }

    SemanticInstruction * inst( new SemanticInstruction(aFetchedOpcode.thePC, aFetchedOpcode.theOpcode, aFetchedOpcode.theBPState, aCPU, aSequenceNo) );

    inst->setClass(clsComputation, codeALU);

    std::vector<std::list<InternalDependance>> rs_deps(3);
    if (opcode == kMoveWideOp_K)
        addReadXRegister(inst, 1, rd, rs_deps[0], sf);
    else
        addReadConstant(inst, 1, 0, rs_deps[0]);

    bits val = 0;
    for (uint8_t i = pos; i <= pos+15; i++){
        val |= (1ULL << i);
    }

    addReadConstant(inst, 2, val, rs_deps[1]);
    addReadConstant(inst, 3, (imm << pos), rs_deps[2]);

    dependant_action wb = writebackAction(inst, kResult, kRD, sf, false);

    predicated_action ow = addExecute(inst, operation(kOVERWRITE_), rs_deps, kResult);
    connectDependance( wb.dependance, ow );


    if (opcode == kMoveWideOp_N){
        predicated_action inv = invertAction(inst, kResult, sf);
        connectDependance( inv.predicate, ow );
        connectDependance( wb.dependance, inv );
    }

    connectDependance( inst->retirementDependance(), wb );
    return inst;
}

arminst LOGICALIMM(armcode const & aFetchedOpcode, uint32_t  aCPU, int64_t aSequenceNo)
{
    DECODER_TRACE;

    uint32_t rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    uint32_t rn = extract32(aFetchedOpcode.theOpcode, 5, 5);
    uint32_t imms = extract32(aFetchedOpcode.theOpcode, 10, 6);
    uint32_t immr = extract32(aFetchedOpcode.theOpcode, 16, 6);
    bool n = extract32(aFetchedOpcode.theOpcode, 22, 1);
    uint32_t opc = extract32(aFetchedOpcode.theOpcode, 29, 2);
    bool sf = extract32(aFetchedOpcode.theOpcode, 31, 1);
    std::unique_ptr<Operation> op;
    bits tmask = 0, wmask = 0;
    bool setflags;

    switch (opc) {
    case 0:
        op = operation(kAND_);
        break;
    case 1:
        op = operation(kORR_);
        break;
    case 2:
        op = operation(kXOR_);
        break;
    case 3:
        setflags = true;
        op = operation(kANDS_);
        break;
    default:
        break;
    }

    if (! decodeBitMasks(tmask, wmask, n, imms, immr, true, sf ? 64 : 32)){
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }

    SemanticInstruction * inst( new SemanticInstruction(aFetchedOpcode.thePC, aFetchedOpcode.theOpcode, aFetchedOpcode.theBPState, aCPU, aSequenceNo) );

    inst->setClass(clsComputation, codeALU);


    std::vector<std::list<InternalDependance>> rs_deps(2);

    addReadXRegister(inst, 1, rn, rs_deps[0], sf);
    addReadConstant(inst, 2, wmask, rs_deps[1]);

    predicated_action exec = addExecute(inst, std::move(op), rs_deps);
    addDestination(inst, rd, exec, sf, setflags);

    return inst;
}

arminst ALUIMM(armcode const & aFetchedOpcode, uint32_t  aCPU, int64_t aSequenceNo)
{
    DECODER_TRACE;
    uint32_t rd = extract32(aFetchedOpcode.theOpcode, 0, 5);
    uint32_t rn = extract32(aFetchedOpcode.theOpcode, 5, 5);
    bits imm = extract32(aFetchedOpcode.theOpcode, 10, 12);
    uint32_t shift = extract32(aFetchedOpcode.theOpcode, 22, 2);
    bool setflags = extract32(aFetchedOpcode.theOpcode, 29, 1);
    bool sub_op = extract32(aFetchedOpcode.theOpcode, 30, 1);
    bool sf = extract32(aFetchedOpcode.theOpcode, 31, 1);

    switch (shift) {
    case 0x0:
        break;
    case 0x1:
        imm <<= 12;
        break;
    default:
        return unallocated_encoding(aFetchedOpcode, aCPU, aSequenceNo);
    }

    SemanticInstruction* inst(new SemanticInstruction(aFetchedOpcode.thePC,aFetchedOpcode.theOpcode,
                                                      aFetchedOpcode.theBPState, aCPU,aSequenceNo));

    inst->setClass(clsComputation, codeALU);

    std::vector<std::list<InternalDependance> > rs_deps(2);
    predicated_action exec = addExecute(inst, operation(setflags ? (sub_op ? kSUBS_ : kADDS_) : (sub_op ? kSUB_ : kADD_)) ,rs_deps);

    addReadXRegister(inst, 1, rn, rs_deps[0], sf);
    addReadConstant(inst, 2, imm, rs_deps[1]);


    addDestination(inst, rd, exec, sf, setflags);

    return inst;

}

} // narmdecoder

