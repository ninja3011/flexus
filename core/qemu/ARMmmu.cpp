//#include <boost/throw_exception.hpp>
//#include <memory>
//#include <functional>

//#include <core/target.hpp>
//#include <core/types.hpp>
//#include <core/qemu/api_wrappers.hpp>

//#include "mai_api.hpp"

//#include <core/qemu/ARMmmu.hpp>

//#include <stdio.h>

//#define DBG_DefineCategories MMUCat
//#define DBG_SetDefaultOps AddCat(MMUCat)
//#include DBG_Control()

//namespace Flexus {
//namespace Qemu {

//#if FLEXUS_TARGET_IS(ARM)
//using namespace Flexus::Qemu::API;

//bool theMMUs_initialized = false;
//std::vector<MMU::mmu_t> theMMUs;
//std::vector<std::deque<MMU::mmu_t> > theMMUckpts;

//std::vector<int> theMMUMap;

//void armProcessorImpl::initializeMMUs() {
//  if (!theMMUs_initialized) {
//    theMMUs_initialized = true;
////    ASI_INFO_DEFINITION(asi_info_table);

//    int num_procs = ProcessorMapper::numProcessors();
//    //int num_procs = SIM_number_processors();
//    theMMUs.resize( num_procs );
//    theMMUckpts.resize( num_procs );

//    theMMUMap.resize( num_procs );

//    for (unsigned int i = 0; i < theMMUs.size(); ++i) {
//      //API::conf_object_t * cpu = Qemu::API::QEMU_get_processor(ProcessorMapper::mapFlexusIndex2ProcNum(i));
//      API::conf_object_t * cpu = Qemu::API::QEMU_get_cpu_by_index(ProcessorMapper::mapFlexusIndex2ProcNum(i));
//      //MMU::fm_init_mmu_from_simics( &theMMUs[i], SIM_get_attribute(cpu, "mmu").u.object );
//      //ALEX - FIXME: Get mmu from QEMU
//    }
//  }
//}

//bool isTranslatingASI(int anASI) {
////  switch (anASI) {
////    case 0:
////      return true;
////    default:
////      return false;
////  }
//    return false;

//}

//long /*opcode*/ armProcessorImpl::fetchInstruction(Translation & aTranslation, bool aTakeTrap) {
//  return fetchInstruction_MMUImpl(aTranslation, aTakeTrap);
//}

//void armProcessorImpl::translate(Translation & aTranslation, bool aTakeException) const {
//  DBG_( Tmp, ( << "MMU: translating... " << VirtualMemoryAddress(aTranslation.theVaddr)) );
//  translate_MMUImpl(aTranslation, aTakeException);
//  DBG_( Tmp, ( << "MMU after translate " << PhysicalMemoryAddress(aTranslation.thePaddr)) );
//}

//unsigned long long armProcessorImpl::readVAddr(VirtualMemoryAddress anAddress, int anASI, int aSize) const {
//  return readVAddr_QemuImpl(anAddress, anASI, aSize);
//}

//unsigned long long armProcessorImpl::readVAddrXendian(Translation & aTranslation, int aSize) const {
//  return readVAddrXendian_MMUImpl(aTranslation, aSize);
//}

//PhysicalMemoryAddress armProcessorImpl::translateInstruction_QemuImpl( VirtualMemoryAddress anAddress) const {
//  try {
//    API::logical_address_t addr(anAddress);
//    API::physical_address_t phy_addr = API::QEMU_logical_to_physical(*this, API::QEMU_DI_Instruction, addr);
//    //checkException();

//    return PhysicalMemoryAddress(phy_addr);
//  } catch (MemoryException & anError ) {

//        DBG_(Tmp, (<<"ARM MMU: Got an Error: " << anError.what()));

//    return PhysicalMemoryAddress(0);
//  }
//}

//long armProcessorImpl::fetchInstruction_QemuImpl(VirtualMemoryAddress const & anAddress) {
//  API::logical_address_t addr(anAddress);
//  API::physical_address_t phy_addr = API::QEMU_logical_to_physical(*this, API::QEMU_DI_Instruction, addr);
//  checkException();

//  long op_code = Qemu::API::QEMU_read_phys_memory( *this, phy_addr, 4);
//  checkException();

//  return op_code;
//}

//bool cacheable(API::arm_memory_transaction_t & xact) {
//  return (xact.cache_virtual && xact.cache_physical );
//}

//bool side_effect(API::arm_memory_transaction_t & xact) {
//  return (xact.side_effect || xact.s.inverse_endian);
//}

////std::tuple<PhysicalMemoryAddress, bool, bool> armProcessorImpl::translateTSB_QemuImpl(VirtualMemoryAddress anAddress) const {
////}

//unsigned long long endianFlip(unsigned long long val, int aSize) {
//  unsigned long long ret_val = 0;
//  switch (aSize) {
//    case 1:
//      ret_val = val;
//      break;
//    case 2:
//      ret_val = ((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8);
//      break;
//    case 4:
//      ret_val = ((val & 0xFF000000) >> 24) | ((val & 0xFF0000) >> 8) | ((val & 0xFF00) << 8) | ((val & 0xFF) << 24);
//      break;
//    case 8:
//      ret_val =
//        ((val & 0xFF00000000000000ULL) >> 56) | ((val & 0xFF000000000000ULL) >> 40) | ((val & 0xFF0000000000ULL) >> 24) | ((val & 0xFF00000000ULL) >> 8) |
//        ((val & 0xFFULL) << 56)               | ((val & 0xFF00ULL) << 40)           | ((val & 0xFF0000ULL) << 24)      | ((val & 0xFF000000ULL) << 8)   ;
//      break;
//    default:
//      DBG_Assert( false, ( << "Unsupported size in endian-flip: " << aSize) );
//  }
//  return ret_val;
//}

//unsigned long long armProcessorImpl::readVAddr_QemuImpl(VirtualMemoryAddress anAddress, int anASI, int aSize) const {
//  try {
//    API::arm_memory_transaction_t xact;
//    translate_QemuImpl( xact, anAddress, anASI );
//    unsigned long long value = Qemu::API::QEMU_read_phys_memory( *this, xact.s.physical_address, aSize);
//    checkException();

//    return value;
//  } catch (MemoryException & anError ) {
//    return 0;
//  }
//}

//unsigned long long armProcessorImpl::readVAddrXendian_QemuImpl(VirtualMemoryAddress anAddress, int anASI, int aSize) const {
//    try {
//        API::logical_address_t addr(anAddress);
//        API::physical_address_t phy_addr = API::QEMU_logical_to_physical(*this, API::QEMU_DI_Data, addr);
//        checkException();
//        unsigned long long value = Qemu::API::QEMU_read_phys_memory( *this, phy_addr, aSize);
//        checkException();
//        return value;
//      } catch (MemoryException & anError ) {}
//}

//void armProcessorImpl::translate_QemuImpl(  API::arm_memory_transaction_t & xact, VirtualMemoryAddress anAddress, int anASI ) const {
//}

//MMU::mmu_t armProcessorImpl::getMMU() {
//  return theMMUs[id()];
//}

//void armProcessorImpl::ckptMMU() {
//  DBG_(Verb, ( << "CPU[" << Qemu::API::QEMU_get_processor_number(*this) << "] checkpointing MMU. size=" << theMMUckpts[id()].size()));
//  theMMUckpts[id()].push_back(theMMUs[id()]);
//}

//void armProcessorImpl::releaseMMUCkpt() {
//  DBG_(Verb, ( << "CPU[" << Qemu::API::QEMU_get_processor_number(*this) << "] releasing oldest MMU checkpoint. size=" << theMMUckpts[id()].size()));
//  DBG_Assert(!theMMUckpts[id()].empty(), ( << "CPU[" << Qemu::API::QEMU_get_processor_number(*this) << "] has no checkpoint to release"));
//  theMMUckpts[id()].pop_front();
//}

//void armProcessorImpl::rollbackMMUCkpts(int n) {
//  DBG_(Verb, ( << "CPU[" << Qemu::API::QEMU_get_processor_number(*this) << "] rolling back " << n << " MMU checkpoints"));
//  DBG_Assert(theMMUckpts[id()].size() > (unsigned)n, ( << "CPU[" << Qemu::API::QEMU_get_processor_number(*this) << "] has " << theMMUckpts[id()].size() << " but needs > " << n));
//  // remove n checkpoints to get back to where we started
//  for (int i = 0; i < n; ++i) theMMUckpts[id()].pop_back();
//  theMMUs[id()] = theMMUckpts[id()].back();
//  theMMUckpts[id()].pop_back();
//}

//void armProcessorImpl::resyncMMU() {
//  //MMU::fm_init_mmu_from_simics(&theMMUs[id()], SIM_get_attribute(*this, "mmu").u.object );	//ALEX - FIXME
//}

//bool armProcessorImpl::validateMMU(MMU::mmu_t * anMMU) {
//  MMU::mmu_t simics_mmu, *our_mmu;
//  //MMU::fm_init_mmu_from_simics(&simics_mmu, SIM_get_attribute(*this, "mmu").u.object);  //ALEX - FIXME
//  assert(false);
//  if (anMMU == NULL) our_mmu = &(theMMUs[id()]);
//  else our_mmu = anMMU;
//  if (MMU::fm_compare_mmus(our_mmu, &simics_mmu)) {
//    return false;
//  }
//  return true;
//}

//unsigned long long armProcessorImpl::mmuRead(VirtualMemoryAddress anAddress) {
//  MMU::mmu_access_t access;
//  access.va = anAddress;
//  access.type = MMU::mmu_access_load;
//  mmu_access( &theMMUs[id()], & access );
//  return access.val;
//}

//void armProcessorImpl::mmuWrite(VirtualMemoryAddress anAddress, /*int anASI,*/ unsigned long long aValue) {
//  MMU::mmu_access_t access;
//  access.va = anAddress;
//  access.type = MMU::mmu_access_store;
//  access.val = aValue;
//  mmu_access( &theMMUs[id()], & access );
//}

//void armProcessorImpl::dumpMMU(MMU::mmu_t * anMMU) {
//  MMU::mmu_t * m = (anMMU == NULL) ? &(theMMUs[id()]) : anMMU;
//  MMU::fm_print_mmu_regs(m);
//}

//void armProcessorImpl::translate_MMUImpl(Translation & aTranslation, bool aTakeException) const {

//    aTranslation.thePaddr = translateInstruction_QemuImpl(aTranslation.theVaddr);
//    DBG_( Tmp, ( <<"\e[33m" << "MMU: " << VirtualMemoryAddress(aTranslation.theVaddr)<< " -> "
//    << PhysicalMemoryAddress(aTranslation.thePaddr)<< "\e[0m")  );
//}

//unsigned long int Endian_DWord_Conversion(unsigned long int dword)//NOOSHIN
//{
//  //return ((dword>>24)&0x000000FF) | ((dword>>8)&0x0000FF00) | ((dword<<8)&0x00FF0000) | ((dword<<24)&0xFF000000);
//    unsigned long int w = __builtin_bswap32 (dword);
//    unsigned long int dw = __builtin_bswap64 (dword);
//    return __builtin_bswap64 (dword);
//}


//long armProcessorImpl::fetchInstruction_MMUImpl(Translation & aTranslation, bool aTakeTrap) {
//  uint64_t op_code = 0;

//  translate_MMUImpl(aTranslation, aTakeTrap );
//  if (aTranslation.thePaddr != 0) {
//    try {
//      op_code = Qemu::API::QEMU_read_phys_memory( *this, aTranslation.thePaddr, 4);
//      //op_code = Endian_DWord_Conversion(op_code);
//      checkException();
//    } catch (...) {

//      DBG_(Tmp,(<<"OPCODE IS GOING TO SET ZEROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO") );
//      op_code = 0;
//    }
//  }

//  DBG_(Tmp, (<< "\e[34m" <<"MMU: Opcode is "<<  std::hex << std::setw(8) << op_code << std::dec << "\e[0m"));
//  return op_code;
//}

//unsigned long long armProcessorImpl::readVAddr_MMUImpl(Translation & aTranslation, int aSize) const {
//  try {
//    translate_MMUImpl(aTranslation, false);

//    DBG_(Tmp, ( << "Virtual: " << aTranslation.theVaddr << /*" ASI: " << aTranslation.theASI <<*/ " Size: " << aSize << " Physical: " << aTranslation.thePaddr ) );//NOOSHIN

//    unsigned long long value = Qemu::API::QEMU_read_phys_memory( *this, aTranslation.thePaddr, aSize);
//    checkException();

//    return value;
//  } catch (MemoryException & anError ) { }
//  return 0;
//}

//unsigned long long armProcessorImpl::readVAddrXendian_MMUImpl(Translation & aTranslation, int aSize) const {
//  try {
//    translate_MMUImpl(aTranslation, false);

//    DBG_(VVerb, ( << "Virtual: " << aTranslation.theVaddr /*<< " ASI: " << aTranslation.theASI */<< " Size: " << aSize << " Physical: " << aTranslation.thePaddr ) );

//    unsigned long long value = Qemu::API::QEMU_read_phys_memory( *this, aTranslation.thePaddr, aSize);
//    checkException();

//    if (aTranslation.isXEndian()) {
//      DBG_(Verb, ( << "Inverse endian access to " << aTranslation.theVaddr  /*<< " ASI: " << aTranslation.theASI*/  << " Size: " << aSize ) );
//      value = endianFlip(value, aSize);
//    }

////    if (aTranslation.theASI == 0x80 && (aTranslation.thePSTATE & 0x100 /*PSTATE.CLE*/)) {
////      DBG_(Verb, ( << "PSTATE.CLE set.  Inverting endianness of  " << aTranslation.theVaddr  << " ASI: " << aTranslation.theASI  << " Size: " << aSize ) );
////      value = endianFlip(value, aSize);
////    }

//    return value;
//  } catch (MemoryException & anError ) { }
//  return 0;
//}

//bool Translation::isCacheable() {
//  return MMU::mmu_is_cacheable( theTTEEntry );
//}
//bool Translation::isSideEffect() {
//  return MMU::mmu_is_sideeffect( theTTEEntry );
//}
//bool Translation::isXEndian() {
//  return MMU::mmu_is_xendian( theTTEEntry );
//}

//bool Translation::isMMU() {
////  switch (theASI) {
////    case 0: //I-MMU
////      return true;

////    default: //all others
////      return false;
////  }
//    return false;
//}
//bool Translation::isInterrupt() {
////  switch (theASI) {
////    case 0:
////      return true;

////    default: //all others
////      return false;
////  }
//    return false;

//}

//bool Translation::isTranslating() {
////  return isTranslatingASI(theASI);
//    return false;
//}

//namespace MMU {

//using namespace Flexus::Qemu::API;

//#define U_BIT_MASK (1ULL<<43)

//static unsigned long long page_mask[4] = {0x1FFF, 0xFFFF, 0x7FFFF, 0x3FFFFF};

//address_t mmu_make_paddr(tte_data data, address_t va) {
//  int size = (data >> 61ULL) & 3;
//  address_t pa = data;
//  pa &= ~0xFFFFF80000000000ULL; /* all things above PA */
//  pa &= ~page_mask[size];
//  pa |= (va & page_mask[size]);
//  return pa;
//}

//#define W_BIT (1ULL<<1)
//#define E_BIT (1ULL<<3)
//#define CV_BIT (1ULL<<4)
//#define CP_BIT (1ULL<<5)
//#define L_BIT (1ULL<<6)
//#define U_BIT (1ULL<<43)
//#define IE_BIT (1ULL<<59)
//#define V_BIT (1ULL<<63)

//#define CTXT_MASK 0x1FFF

//bool mmu_is_cacheable(tte_data data) {
//  return (!! (data & CV_BIT)) && (!! (data & CP_BIT));
//}

//bool mmu_is_sideeffect(tte_data data) {
//  return !!(data & E_BIT);
//}

//bool mmu_is_xendian(tte_data data) {
//  return !!(data & IE_BIT);
//}

//bool mmu_is_writeable(tte_data data) {
//  return !!(data & W_BIT);
//}

//address_t
//mmu_translate(mmu_t * mmu,
//              unsigned int is_fetch,
//              address_t va,
//              unsigned int klass,
//              unsigned int nofault,
//              unsigned int priv,
//              unsigned int access_type,
//              mmu_exception_t * except,
//              mmu_translation_type_t trans_type) {
//  tte_data data = mmu_lookup(mmu, is_fetch, va, klass, nofault, priv, access_type, except, trans_type);
//  if (*except != no_exception) {
//    return 0;
//  }
//  return mmu_make_paddr(data, va);
//}

//tte_data
//mmu_lookup(mmu_t * mmu,
//           unsigned int is_fetch,
//           address_t va,
//           unsigned int klass,
//           unsigned int nofault,
//           unsigned int priv,
//           unsigned int access_type,
//           mmu_exception_t * except,
//           mmu_translation_type_t trans_type) {
//  mmu_reg_t ctxt = 0;
//  mmu_reg_t test_ctxt, test_global;
//  mmu_reg_t hit_data = 0;
//  mmu_reg_t hit_test_ctxt = 0, hit_test_global = 0;
//  int way, hit_flag, hit_size = 0;
//  mmu_reg_t tsb_ext = 0;

//  tte_tag * tag_2w, *data_2w, *tag_fa, *data_fa;
//  tlb_regs_t * regs;
//  int sets_2w, ways_fa;
//  address_t tsb_base, tsb_reg;
//  unsigned int tsb_split, tsb_size;
//  *except = no_exception;

//  /* prelims */
//  if (is_fetch) {
//    sets_2w = 64;
//    ways_fa = 16;
//    tag_2w  = mmu->it128_tag;
//    data_2w = mmu->it128_data;
//    tag_fa  = mmu->it16_tag;
//    data_fa = mmu->it16_data;
//    regs    = &(mmu->i_regs);
//  } else {
//    sets_2w = 256;
//    ways_fa = 16;
//    tag_2w  = mmu->dt512_tag;
//    data_2w = mmu->dt512_data;
//    tag_fa  = mmu->dt16_tag;
//    data_fa = mmu->dt16_data;
//    regs    = &(mmu->d_regs);
//  }

//  /* procedure:
//   *  1. get context from appropriate context reg (use class)
//   *  2. search 2-way TLB
//   *       if hit, set PA
//   *  3.   else miss, search FA TLB
//   *         if hit, set PA
//   *  4.     else miss, set exception
//   */

//  /* get context */
//  switch (klass) {
//    case CLASS_ASI_PRIMARY:
//      ctxt = mmu->primary_context;
//      tsb_ext = regs->tsb_px;
//      break;
//    case CLASS_ASI_SECONDARY:
//      ctxt = mmu->secondary_context;
//      tsb_ext = regs->tsb_sx;
//      break;
//    case CLASS_ASI_NUCLEUS:
//      ctxt = 0;  /* hardwired */
//      tsb_ext = regs->tsb_nx;
//      break;
//    default:
//      /* TODO: set exception? */
//      break;
//  }

//  DBG_( Verb,  ( << "DBG: context = 0x" << ctxt));

//  hit_flag = 0;

//  /* 2. look in 2-way table */
//  for (way = 0; way < 2; way++) {
//    int idx = sets_2w * way + ((va >> 13ULL) % sets_2w); /* FIXME: may not work w/ non-8K pages */
//    int size = (data_2w[idx] >> 61ULL) & 3;
//    //test_ctxt   = (ctxt << 48ULL) | (va & ~page_mask[size]);
//    test_ctxt   = (ctxt) | (va & ~page_mask[size]);
//    test_global = (1ULL << 63ULL) | (va & ~page_mask[size]);
//    DBG_( VVerb, ( << " idx=" << idx << " tag_2w=" << std::hex << tag_2w[idx] << " data_2w=" << data_2w[way] << " test_ctxt=" << test_ctxt << " test_global=" << test_global << std::dec ) );
//    if (tag_2w[idx] == test_ctxt || tag_2w[idx] == test_global) {
//      /* hit! */
//      hit_flag = 1;
//      hit_data = data_2w[idx];
//      hit_test_ctxt   = test_ctxt;
//      hit_test_global = test_global;

//      if (trans_type == MMU_DEMAP_PAGE) {
//        tag_2w[idx] = 0;
//        data_2w[idx] &= ~(1ULL << 63);
//      } else {
//        if (trans_type == MMU_TRANSLATE) data_2w[idx] |= U_BIT_MASK;
//        break;
//      }
//    }
//  }

//  /* 3. look in FA table */
//  if (!hit_flag || trans_type == MMU_DEMAP_PAGE) {
//    for (way = 0; way < ways_fa; way++) {
//      int size = (data_fa[way] >> 61ULL) & 3;
//      //test_ctxt   = (ctxt << 48ULL) | (va & ~page_mask[size]);
//      test_ctxt   = (ctxt) | (va & ~page_mask[size]);
//      test_global = (1ULL << 63ULL) | (va & ~page_mask[size]);
//      DBG_( VVerb, ( << " way=" << way << " tag_fa=" << std::hex << tag_fa[way] << " data_fa=" << data_fa[way] << " test_ctxt=" << test_ctxt << " test_global=" << test_global << std::dec ) );
//      if (tag_fa[way] == test_ctxt || tag_fa[way] == test_global) {
//        /* hit! */
//        hit_flag = 1;
//        hit_data = data_fa[way];
//        hit_test_ctxt   = test_ctxt;
//        hit_test_global = test_global;

//        if (trans_type == MMU_DEMAP_PAGE) {
//          tag_fa[way] = 0;
//          data_fa[way] &= ~(1ULL << 63);
//        } else {
//          if (trans_type == MMU_TRANSLATE) data_fa[way] |= U_BIT_MASK;
//          break;
//        }
//      }
//    }
//  }

//  /* fill in return details */
//  if (!hit_flag) {
//    if (trans_type == MMU_TRANSLATE || trans_type == MMU_TRANSLATE_PF) {
//      unsigned int ow;
//      /* take actions on a TLB miss */

//      //FIXME - add a way for I-fetch accesses to probe without causing trap
//      if (is_fetch) {
//        DBG_(Tmp, ( << "I-TLB miss to va: " << std::hex << va << std::dec ));//NOOSHIN

//        *except = fast_instruction_access_MMU_miss;

//        if (trans_type == MMU_TRANSLATE) {

//          /* update registers */
//          mmu->i_regs.tsb_tag_target = (ctxt << 48) | (va >> 22);
//          mmu->i_regs.tag_access = (va & ~0x1FFF) | ctxt;

//          tsb_reg   =  mmu->i_regs.tsb ^ tsb_ext;
//          tsb_base  =  tsb_reg & ~0x1FFF;
//          tsb_split = (tsb_reg & 0x1000) >> 12;
//          tsb_size  =  tsb_reg & 0x7;
//          mmu->i_regs.tsbp8k  = mmu_generate_tsb_ptr(va, MMU_TSB_8K_PTR, tsb_base,
//                                tsb_split, tsb_size, tsb_ext);
//          mmu->i_regs.tsbp64k = mmu_generate_tsb_ptr(va, MMU_TSB_64K_PTR, tsb_base,
//                                tsb_split, tsb_size, tsb_ext);

//          /* update SFSR -- requires various state info */
//          ow  = (mmu->i_regs.sfsr & 0x2) >> 1;  /* set if fv still set */
//          mmu->i_regs.sfsr = (nofault << 24)
////                             | (asi << 16)
//                             | (is_fetch << 15)
//                             | (klass << 4)
//                             | (priv << 3)
//                             | (ow << 1)
//                             | (0 /* fv */ );
//        }
//      } else {
//        *except = fast_data_access_MMU_miss;

//        if (trans_type == MMU_TRANSLATE) {
//          /* update registers */
//          mmu->d_regs.tsb_tag_target = (ctxt << 48) | (va >> 22);
//          mmu->d_regs.sfar = va;
//          mmu->d_regs.tag_access = (va & ~0x1FFF) | ctxt;

//          /* generate tsb pointers */
//          tsb_reg   =  mmu->d_regs.tsb ^ tsb_ext;
//          tsb_base  =  tsb_reg & ~0x1FFF;
//          tsb_split = (tsb_reg & 0x1000) >> 12;
//          tsb_size  =  tsb_reg & 0x7;
//          mmu->d_regs.tsbp8k  = mmu_generate_tsb_ptr(va, MMU_TSB_8K_PTR, tsb_base,
//                                tsb_split, tsb_size, tsb_ext);
//          mmu->d_regs.tsbp64k = mmu_generate_tsb_ptr(va, MMU_TSB_64K_PTR, tsb_base,
//                                tsb_split, tsb_size, tsb_ext);

//          /* update SFSR -- requires various state info */
//          ow  = (mmu->d_regs.sfsr & 0x2) >> 1;  /* set if fv still set */
//          mmu->d_regs.sfsr = (nofault << 24)
////                             | (asi << 16)
//                             | (!is_fetch << 15)
//                             | (klass << 4)
//                             | (priv << 3)
//                             | ((access_type != 1 /* FIXME: load */)  << 2)
//                             | (ow << 1)
//                             | (1 /* fv */ );
//        }
//      }
//    }
//  } else {
//    /* check for protection violations */
//    if (access_type == mmu_access_store && !mmu_is_writeable(hit_data)) {
//      *except = fast_data_access_protection;

//      if (trans_type == MMU_TRANSLATE) {
//        unsigned int ow;
//        /* update registers */
//        mmu->d_regs.tsb_tag_target = (ctxt << 48) | (va >> 22);
//        mmu->d_regs.sfar = va;
//        mmu->d_regs.tag_access = (va & ~0x1FFF) | ctxt;

//        /* generate tsb pointers */
//        tsb_reg   =  mmu->d_regs.tsb ^ tsb_ext;
//        tsb_base  =  tsb_reg & ~0x1FFF;
//        tsb_split = (tsb_reg & 0x1000) >> 12;
//        tsb_size  =  tsb_reg & 0x7;
//        mmu->d_regs.tsbp8k  = mmu_generate_tsb_ptr(va, MMU_TSB_8K_PTR, tsb_base,
//                              tsb_split, tsb_size, tsb_ext);
//        mmu->d_regs.tsbp64k = mmu_generate_tsb_ptr(va, MMU_TSB_64K_PTR, tsb_base,
//                              tsb_split, tsb_size, tsb_ext);

//        /* update SFSR -- requires various state info */
//        ow  = (mmu->d_regs.sfsr & 0x2) >> 1;  /* set if fv still set */
//        mmu->d_regs.sfsr = (nofault << 24)
//                           | (asi << 16)
//                           | (!is_fetch << 15)
//                           | (klass << 4)
//                           | (priv << 3)
//                           | ((access_type != 1 /* FIXME: load */)  << 2)
//                           | (ow << 1)
//                           | (1 /* fv */ );
//      }
//    }
//  }

//  DBG_( Tmp, ( << "MMU va: " << std::hex << va << " is_fetch: " << is_fetch << " hit: " << hit_flag << " test_ctxt: " << hit_test_ctxt << " test_global " << hit_test_global << " hit_data: " << hit_data << " size: " << hit_size ) ); //NOOSHIN

//  return hit_data;
//}

//address_t
//mmu_generate_tsb_ptr(address_t va,
//                     mmu_ptr_type_t type,
//                     address_t tsb_base_in,
//                     unsigned int tsb_split,
//                     unsigned int tsb_size,
//                     mmu_reg_t tsb_ext) {
//  address_t tsb_base = tsb_base_in;
//  address_t va_portion;
//  address_t tsb_base_mask;
//  address_t split_mask;

//  va_portion = (va >> ((type == MMU_TSB_8K_PTR) ? 9 : 12)) & 0xFFFFFFFFFFFFFFF0ULL;

//  //tsb_base ^= tsb_ext; -- performed before calling mmu_generate_tsb_ptr
//  va_portion ^= tsb_ext << 1 & 0x1ff0;
//  va_portion ^= tsb_ext & 0x1fe000;

//  tsb_base_mask = 0xFFFFFFFFFFFFe000ULL << (tsb_split ? (tsb_size + 1) : tsb_size);

//  if (tsb_split) {
//    split_mask = 1 << (13 + tsb_size);
//    if (type == MMU_TSB_8K_PTR) {
//      va_portion &= ~split_mask;
//    } else {
//      va_portion |= split_mask;
//    }
//  }
//  return (tsb_base & tsb_base_mask) | (va_portion & ~tsb_base_mask);
//}

//int mmu_fa_choose(mmu_t * mmu ) {
//  //First invalid entry
//  int i;
//  for (i = 0; i < 16; ++i) {
//    if ((mmu->dt16_data[i] & V_BIT) == 0) {
//      return i;
//    }
//  }

//  //First unlocked, unused entry
//  for (i = 0; i < 16; ++i) {
//    if (((mmu->dt16_data[i] & L_BIT) == 0) && ((mmu->dt16_data[i] & U_BIT) == 0)) {
//      return i;
//    }
//  }

//  //Clear all used bits
//  for (i = 0; i < 16; ++i) {
//    mmu->dt16_data[i] &= ~ U_BIT;
//  }

//  //First unlocked entry
//  for (i = 0; i < 16; ++i) {
//    if ((mmu->dt16_data[i] & L_BIT) == 0) {
//      return i;
//    }
//  }

//  //Undefined behavior if all entries locked.
//  DBG_Assert( false, ( << "All entries in fully-associative TLB are locked" ) );
//  return 0;
//}

//void mmu_access(mmu_t * mmu, mmu_access_t * access) {

//    assert(false);
//}

//#define FM_GET_BITS(val, msb, lsb) ((val >> lsb) & ((1ULL<<(msb-lsb+1)) - 1))

//void
//fm_print_mmu_regs(mmu_t * mmu) {
//  tlb_regs_t * d_regs = &(mmu->d_regs);
//  tlb_regs_t * i_regs = &(mmu->i_regs);

//  printf("Context registers:\n");
//  printf("  Primary ctxt:   0x%04llx ", mmu->primary_context);
//  printf("  Secondary ctxt: 0x%04llx ", mmu->secondary_context);
//  printf("  Nucleus ctxt:   0x0000\n");

//  printf("LSU control register: 0x000000000000000f\n");
//  printf("  D-MMU enable:   true    D-cache enable: true\n");
//  printf("  I-MMU enable:   true    I-cache enable: true\n");

//  printf("D-MMU sync fault status register:\n");
//  printf("  asi: 0x%02llx  ft: 0x%02llx  e: %1llx  ct: %1llx  pr: %1llx  w: %1llx ow: %1llx fv: %1llx\n",
//         FM_GET_BITS(d_regs->sfsr, 23, 16), /* ASI */
//         FM_GET_BITS(d_regs->sfsr, 11, 7), /* FT  */
//         FM_GET_BITS(d_regs->sfsr, 6, 6),  /* E   */
//         FM_GET_BITS(d_regs->sfsr, 5, 4),  /* CT  */
//         FM_GET_BITS(d_regs->sfsr, 3, 3),  /* PR  */
//         FM_GET_BITS(d_regs->sfsr, 2, 2),  /* W   */
//         FM_GET_BITS(d_regs->sfsr, 1, 1),  /* OW  */
//         FM_GET_BITS(d_regs->sfsr, 0, 0)); /* FV  */

//  printf("I-MMU sync fault status register:\n");
//  printf("  asi: 0x%02llx  ft: 0x%02llx  e: %1llx  ct: %1llx  pr: %1llx  w: %1llx ow: %1llx fv: %1llx\n",
//         FM_GET_BITS(i_regs->sfsr, 23, 16), /* ASI */
//         FM_GET_BITS(i_regs->sfsr, 11, 7), /* FT  */
//         FM_GET_BITS(i_regs->sfsr, 6, 6),  /* E   */
//         FM_GET_BITS(i_regs->sfsr, 5, 4),  /* CT  */
//         FM_GET_BITS(i_regs->sfsr, 3, 3),  /* PR  */
//         FM_GET_BITS(i_regs->sfsr, 2, 2),  /* W   */
//         FM_GET_BITS(i_regs->sfsr, 1, 1),  /* OW  */
//         FM_GET_BITS(i_regs->sfsr, 0, 0)); /* FV  */

//  printf("D-MMU sync fault address register:\n");
//  printf("  va: 0x%016llx\n", d_regs->sfar);

//  printf("D-MMU tag access register:\n");
//  printf("  va: 0x%016llx  0x%04llx\n",
//         d_regs->tag_access & (~0x1FFFULL),
//         FM_GET_BITS(d_regs->tag_access, 12, 0));

//  printf("D-MMU tag access register:\n");
//  printf("  va: 0x%016llx  0x%04llx\n",
//         i_regs->tag_access & (~0x1FFFULL),
//         FM_GET_BITS(i_regs->tag_access, 12, 0));

//  printf("D tsbp8k     0x%016llx    I tsbp8k     0x%016llx\n", d_regs->tsbp8k,         i_regs->tsbp8k        );
//  printf("D tsbp64k    0x%016llx    I tsbp64k    0x%016llx\n", d_regs->tsbp64k,        i_regs->tsbp64k       );
//  printf("D tsbpd      0x%016llx                          \n", d_regs->tsbpd                                 );
//  printf("D tsb        0x%016llx    I tsb        0x%016llx\n", d_regs->tsb,            i_regs->tsb           );
//  printf("D tag_target 0x%016llx    I tag_target 0x%016llx\n", d_regs->tsb_tag_target, i_regs->tsb_tag_target);
//  printf("D tsb_px     0x%016llx    I tsb_px     0x%016llx\n", d_regs->tsb_px,         i_regs->tsb_px        );
//  printf("D tsb_sx     0x%016llx                          \n", d_regs->tsb_sx                                );
//  printf("D tsb_nx     0x%016llx    I tsb_nx     0x%016llx\n", d_regs->tsb_nx,         i_regs->tsb_nx        );

//}

////ALEX - FIXME: Pair the Flexus mmu with the QEMU mmu
///*
//#define FM_COPY_FROM_SIMICS(ours, simics) \
//do { attr_value_t val = SIM_get_attribute(chmmu, simics); \
//     ours = val.u.integer; } while(0);

//#define FM_COPY_LIST_FROM_SIMICS(ours, simics) \
//do { attr_value_t val = SIM_get_attribute(chmmu, simics); \
//     int i, size = val.u.list.size; \
//     for (i = 0; i < size; i++) { \
//       ours[i] = val.u.list.vector[i].u.integer; \
//     } SIM_free_attribute(val); } while(0);
//*/
///*
// * fm_init_mmu_from_simics: initializes an MMU to Simics MMU state
// */
///*
//void
//fm_init_mmu_from_simics(mmu_t * mmu, conf_object_t * chmmu) {
//  tlb_regs_t * d_regs = &(mmu->d_regs);
//  tlb_regs_t * i_regs = &(mmu->i_regs);

//  FM_COPY_FROM_SIMICS(mmu->primary_context,      "ctxt_primary");
//  FM_COPY_FROM_SIMICS(mmu->secondary_context,    "ctxt_secondary");

//  FM_COPY_FROM_SIMICS(d_regs->sfar,              "dsfar");
//  FM_COPY_FROM_SIMICS(d_regs->sfsr,              "dsfsr");
//  FM_COPY_FROM_SIMICS(d_regs->tag_access,        "dtag_access");
//  FM_COPY_FROM_SIMICS(d_regs->tsb_tag_target,    "dtag_target");
//  FM_COPY_FROM_SIMICS(d_regs->tsb,               "dtsb");
//  FM_COPY_FROM_SIMICS(d_regs->tsb_nx,            "dtsb_nx");
//  FM_COPY_FROM_SIMICS(d_regs->tsb_px,            "dtsb_px");
//  FM_COPY_FROM_SIMICS(d_regs->tsb_sx,            "dtsb_sx");
//  FM_COPY_FROM_SIMICS(d_regs->tsbp64k,           "dtsbp64k");
//  FM_COPY_FROM_SIMICS(d_regs->tsbp8k,            "dtsbp8k");
//  FM_COPY_FROM_SIMICS(d_regs->tsbpd,             "dtsbpd");

//  FM_COPY_FROM_SIMICS(i_regs->sfsr,              "isfsr");
//  FM_COPY_FROM_SIMICS(i_regs->tag_access,        "itag_access");
//  FM_COPY_FROM_SIMICS(i_regs->tsb_tag_target,    "itag_target");
//  FM_COPY_FROM_SIMICS(i_regs->tsb,               "itsb");
//  FM_COPY_FROM_SIMICS(i_regs->tsb_nx,            "itsb_nx");
//  FM_COPY_FROM_SIMICS(i_regs->tsb_px,            "itsb_px");
//  FM_COPY_FROM_SIMICS(i_regs->tsbp64k,           "itsbp64k");
//  FM_COPY_FROM_SIMICS(i_regs->tsbp8k,            "itsbp8k");
//  i_regs->sfar   = (mmu_reg_t)(-1);
//  i_regs->tsb_sx = (mmu_reg_t)(-1);
//  i_regs->tsbpd  = (mmu_reg_t)(-1);

//  FM_COPY_FROM_SIMICS(mmu->lfsr,                 "lfsr");

//  FM_COPY_FROM_SIMICS(mmu->pa_watchpoint,        "pa_watchpoint");
//  FM_COPY_FROM_SIMICS(mmu->va_watchpoint,        "va_watchpoint");

//  FM_COPY_LIST_FROM_SIMICS(mmu->dt16_tag,        "dtlb_fa_tagread");
//  FM_COPY_LIST_FROM_SIMICS(mmu->dt16_data,       "dtlb_fa_daccess");
//  FM_COPY_LIST_FROM_SIMICS(mmu->dt512_tag,       "dtlb_2w_tagread");
//  FM_COPY_LIST_FROM_SIMICS(mmu->dt512_data,      "dtlb_2w_daccess");

//  FM_COPY_LIST_FROM_SIMICS(mmu->it16_tag,        "itlb_fa_tagread");
//  FM_COPY_LIST_FROM_SIMICS(mmu->it16_data,       "itlb_fa_daccess");
//  FM_COPY_LIST_FROM_SIMICS(mmu->it128_tag,       "itlb_2w_tagread");
//  FM_COPY_LIST_FROM_SIMICS(mmu->it128_data,      "itlb_2w_daccess");

//}
//*/
//#define FM_COMPARE_FIELD(who, field) do { \
//  if (a->field != b->field) { \
//   DBG_(Crit, (<< "Mismatch in " << who << "->" << #field)); \
//  DBG_(Crit, (<< " us:        " << std::hex << a->field)); \
//  DBG_(Crit, (<< " simics:    " << std::hex << b->field)); \
//    mismatch = 1; } } while(0);

//#define FM_COMPARE_ARRAY(a, b, len, who) do { \
//  int i; \
//  for (i = 0; i < len; i++) { \
//    if (a[i] != b[i]) { \
//   DBG_(Crit, (<< "Mismatch in " << who << "[" << i << "]")); \
//   DBG_(Crit, (<< " us:        " << std::hex << a[i])); \
//   DBG_(Crit, (<< " simics:    " << std::hex << b[i])); \
//      mismatch = 1; } } } while(0);

///*
// * fm_compare_regs: compare d- or i-tlb registers
// */
//int
//fm_compare_regs(tlb_regs_t * a, tlb_regs_t * b, const char * who) {
//  int mismatch = 0;

//  FM_COMPARE_FIELD(who, tsb_tag_target);
//  //FM_COMPARE_FIELD(who, sfsr);
//  FM_COMPARE_FIELD(who, sfar);
//  FM_COMPARE_FIELD(who, tsb);
//  FM_COMPARE_FIELD(who, tag_access);
//  FM_COMPARE_FIELD(who, tsb_px);
//  FM_COMPARE_FIELD(who, tsb_sx);
//  FM_COMPARE_FIELD(who, tsb_nx);
//  FM_COMPARE_FIELD(who, tsbp8k);
//  FM_COMPARE_FIELD(who, tsbp64k);
//  //FM_COMPARE_FIELD(who, tsbpd);
//  /*FM_COMPARE_FIELD(who, data_in);  not used in simics */

//  return mismatch;
//}

///* fm_compare_mmus: compares two MMUs and prints any differences */
//int
//fm_compare_mmus(mmu_t * a, mmu_t * b) {
//  int mismatch = 0;

//  FM_COMPARE_FIELD("mmu", primary_context);
//  FM_COMPARE_FIELD("mmu", secondary_context);
//  FM_COMPARE_FIELD("mmu", pa_watchpoint);
//  FM_COMPARE_FIELD("mmu", va_watchpoint);

//  mismatch |= fm_compare_regs(&(a->d_regs), &(b->d_regs), "mmu->d_regs");
//  mismatch |= fm_compare_regs(&(a->i_regs), &(b->i_regs), "mmu->i_regs");

//  FM_COMPARE_FIELD("mmu", lfsr);

//  FM_COMPARE_ARRAY(a->dt16_tag,   b->dt16_tag,   16,  "mmu->dt16_tag");
//  FM_COMPARE_ARRAY(a->dt16_data,  b->dt16_data,  16,  "mmu->dt16_data");
//  FM_COMPARE_ARRAY(a->dt512_tag,  b->dt512_tag,  512, "mmu->dt512_tag");
//  FM_COMPARE_ARRAY(a->dt512_data, b->dt512_data, 512, "mmu->dt512_data");

//  FM_COMPARE_ARRAY(a->it16_tag,   b->it16_tag,   16,  "mmu->it16_tag");
//  FM_COMPARE_ARRAY(a->it16_data,  b->it16_data,  16,  "mmu->it16_data");
//  FM_COMPARE_ARRAY(a->it128_tag,  b->it128_tag,  128, "mmu->it128_tag");
//  FM_COMPARE_ARRAY(a->it128_data, b->it128_data, 128, "mmu->it128_data");

//  return mismatch;
//}

//}
//} //end Namespace Simics
//} //end namespace Flexus

//#endif // IS_ARM
