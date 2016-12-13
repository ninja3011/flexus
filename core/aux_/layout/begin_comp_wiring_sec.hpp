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
#ifndef FLEXUS__CORE_TEST

#ifdef FLEXUS__LAYOUT_COMPONENTS_WIRED
#error "Wiring.cpp may contain only one component wiring section"
#endif //FLEXUS__LAYOUT_COMPONENTS_WIRED

#ifndef FLEXUS__LAYOUT_COMPONENTS_INSTANTIATED
#error "The component wiring section of wiring.cpp must follow the component instantiation section"
#endif //FLEXUS__LAYOUT_COMPONENTS_INSTANTIATED

#ifdef FLEXUS__LAYOUT_IN_SECTION
#error "Previous wiring.cpp section is missing the end of section #include"
#endif //FLEXUS__LAYOUT_IN_SECTION

#define FLEXUS__LAYOUT_IN_SECTION
#define FLEXUS__LAYOUT_COMPONENT_WIRING_SECTION

#endif //FLEXUS__CORE_TEST

#ifdef FLEXUS__CORE_TEST
#define nFLEXUS FLEXUS__CORE_TEST
#else
#define nFLEXUS Flexus
#endif //FLEXUS__CORE_TEST

namespace nFLEXUS {
namespace Wiring {

#define WIRE( FromInstance, FromPort, ToInstance, ToPort)                                                                                   \
    BOOST_PP_CAT(FromInstance,_instance).theJumpTable.BOOST_PP_CAT(wire_available_,FromPort)                                                \
      = & resolve_channel                                                                                                                   \
            < ToInstance                                                                                                                    \
            , ToInstance::iface::ToPort                                                                                                     \
            , ToInstance::iface::ToPort::port_type                                                                                          \
            , ToInstance::iface::ToPort::is_array                                                                                           \
            >::invoke_available;                                                                                                            \
    BOOST_PP_CAT(FromInstance,_instance).theJumpTable.BOOST_PP_CAT(wire_manip_,FromPort)                                                    \
      = & resolve_channel                                                                                                                   \
            < ToInstance                                                                                                                    \
            , ToInstance::iface::ToPort                                                                                                     \
            , ToInstance::iface::ToPort::port_type                                                                                          \
            , ToInstance::iface::ToPort::is_array                                                                                           \
            >::invoke_manip;                                                                                                                /**/

void connectWiring() {

