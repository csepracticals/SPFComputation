/*
 * =====================================================================================
 *
 *       Filename:  igp_sr_ext.h
 *
 *    Description:  HEader defining common SR definitions
 *
 *        Version:  1.0
 *        Created:  Saturday 24 February 2018 11:32:36  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *        
 *        This file is part of the SPFComputation distribution (https://github.com/sachinites).
 *        Copyright (c) 2017 Abhishek Sagar.
 *        This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by  
 *        the Free Software Foundation, version 3.
 *
 *        This program is distributed in the hope that it will be useful, but 
 *        WITHOUT ANY WARRANTY; without even the implied warranty of 
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 *        General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License 
 *        along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================================
 */

#ifndef __IGP_SR_EXT__
#define __IGP_SR_EXT__

#include "instanceconst.h"

#define SHORTEST_PATH_FIRST  0 
#define STRICT_SHORTEST_PATH 1


/*SRGB constants*/
#define SRGB_DEF_LOWER_BOUND    16000
#define SRGB_DEF_UPPER_BOUND    23999
#define SRGB_MAX_SIZE           65536
    

/*TLV types*/
#define PREFIX_SID_SUBTLV_TYPE          3
#define SID_SUBTLV_TYPE                 1
#define SR_CAPABILITY_SRGB_SUBTLV_TYPE  2
#define SR_CAPABILITY_SRLB_SUBTLV_TYPE  22


#define PREFIX_SID_VALUE(prefix_ptr)    (prefix_ptr->prefix_sid->sid.sid)

/*Segment ID SUB-TLV structure
 * contains a SID or a MPLS Label*/
typedef struct _segment_id_subtlv_t{
    BYTE type;      /*constant = 1*/
    BYTE length;    /*if 3, then 20 rightmost bit represents label, if 4 then index into srgb*/
    //char *sid;    /*RFC compliant*/
    unsigned int sid; /*our implementation compliant*/
} segment_id_subtlv_t; 

/* SR Capability TLV FLAGS*/

/*If set, then the router is capable of
processing SR MPLS encapsulated IPv4 packets on all interfaces*/
#define SR_CAPABILITY_I_FLAG    7

/* If set, then the router is capable of
 * processing SR MPLS encapsulated IPv6 packets on all interfaces.
 * */
#define SR_CAPABILITY_V_FLAG    6

typedef struct routes_ routes_t;

/*Used to advertise the srgb block*/
typedef struct _sr_capability_subtlv{

   BYTE type;      /*constant = 2 for SRGB. 22 for SRLB*/ 
   BYTE length;
   BYTE flags;      /*Not defined for SRLB*/
   /*The below two members could be repeated multiple times
    * We can iterate through them using length*/
   unsigned int range;  /*only last three bytes, must > 0*/   
   segment_id_subtlv_t first_sid; /*SID/Label sub-TLV contains the first value of the SRGB while the
   range contains the number of SRGB elements*/
   /*Out of RFC  :Bits to track the index allocation*/
   char *index_array;
} sr_capability_subtlv_t;

typedef sr_capability_subtlv_t srgb_t;
typedef sr_capability_subtlv_t srlb_t;

mpls_label_t
get_available_srgb_mpls_label(srgb_t *srgb);

void
init_srgb_defaults(srgb_t *srgb);

void
mark_srgb_mpls_label_in_use(srgb_t *srgb, mpls_label_t label);

void
mark_srgb_mpls_label_not_in_use(srgb_t *srgb, mpls_label_t label);

boolean
is_mpls_label_in_use(srgb_t *srgb, mpls_label_t label);

/*SR algorithm TLV
 * allows the router to advertise the algorithms
 * that the router is currently using*/
typedef struct _sr_algorithm{
   BYTE type;
   BYTE length;
   BYTE algos[0]; /*list of algos supported*/ 
} sr_algorithm_subtlv_t;

/*Router capability TLV*/
typedef struct _router_cap_tlv{
    BYTE type;
    BYTE length;
    // ...
    srgb_t *srgb;
    sr_algorithm_subtlv_t sr_algo;
    srlb_t *srlb;
} router_cap_subtlv_t;

/*PREFIX SID flags*/
/*
   If set, then the prefix to
   which this Prefix-SID is attached, has been propagated by the
   router either from another level (i.e., from level-1 to level-2
   or the opposite) or from redistribution (e.g.: from another
   protocol). Note that Router MUST NOT be a original 
   hosting node of the prefix. The R-bit MUST be set only for prefixes 
   that are not local to the router and advertised by the router 
   because of propagation and/or leaking.
*/
#define RE_ADVERTISEMENT_R_FLAG   7

/*
 * Set if the attached prefix is a local prefix of the router AND
 *        its mask length is /32 or /128.
 * Error handling : If remote router recieved the prefix TLV with
 * N FLAG set but mask len is not /32(/128), then routr MUST ignore
 * N flag
 * */
#define NODE_SID_N_FLAG           6

/* If set, then the penultimate hop MUST NOT
 * pop the Prefix-SID before delivering the packet to the node
 * that advertised the Prefix-SID.
 * The IGP signaling extension for IGP-Prefix segment includes a flag
 * to indicate whether directly connected neighbors of the node on
 * which the prefix is attached should perform the NEXT operation or
 * the CONTINUE operation when processing the SID. This behavior is
 * equivalent to Penultimate Hop Popping (NEXT) or Ultimate Hop
 * Popping (CONTINUE) in MPLS.
 * pg 10 - Segment Routing Architecture-2
 * When a non-local router leak the prefix(in any dirn), Router MUST
 * set P flag*/
#define NO_PHP_P_FLAG             5

/* This flag is processed only when P flag is set. If E flag is set, 
 * any upstream neighbor of
 * the Prefix-SID originator MUST replace the Prefix-SID with a
 * Prefix-SID having an Explicit-NULL value (0 for IPv4 and 2 for
 * IPv6) before forwarding the packet.
 * If E flag is not set, then any upstream neighbor of the
 * Prefix-SID originator MUST keep the Prefix-SID on top of the
 * stack.
 * If the E-flag is set then any upstream neighbor of the Prefix-
 * SID originator MUST replace the PrefixSID with a Prefix-SID
 * having an Explicit-NULL value. This is useful, e.g., when the
 * originator of the Prefix-SID is the final destination for the
 * related prefix and the originator wishes to receive the packet
 * with the original EXP bits.
 * This flag is unset when non local router leaks the prefix (in any dirn)
 * If the P-flag is unset the received E-flag is ignored.
 * */
#define EXPLICIT_NULL_E_FLAG      4

/* If set, then the Prefix-SID carries a
 * value (instead of an index). By default the flag is UNSET.
 * */
#define VALUE_V_FLAG              3

/* If set, then the value/index carried by
 * the Prefix-SID has local significance. By default the flag is
 * UNSET.
 * */
#define LOCAL_SIGNIFICANCE_L_FLAG 2

typedef struct _sr_mapping_entry_t sr_mapping_entry_t;

typedef enum{
    SID_ACTIVE,
    SID_INACTIVE
} conflict_result_t;

/*prefix SID */
typedef struct _prefix_sid_subtlv_t{

    BYTE type;  /*constant = 3*/
    BYTE length;
    BYTE flags; /*0th bit = not used, 1st bit = unused, 2 bit = L, 3rd bit = V, 4th bit = E, 5th bit = P, 6th bit = N; 7th bit = R*/
    BYTE algorithm; /*0 value - Shortest Path First, 1 value - strict Shortest Path First*/
    segment_id_subtlv_t sid; /*Index into srgb block. V and L bit are not set in this case Or
                        3 byte value whose 20 right most bits are used for encoding label value. V and L bits are set in this case.
                        This field can be eithther the SID value Or index into SRGB or absolute MPLS label value*/
    /* The IGP signaling extension for IGP-Prefix segment includes a flag
     * to indicate whether directly connected neighbors of the node on
     * which the prefix is attached should perform the NEXT operation or
     * the CONTINUE operation when processing the SID. This behavior is
     * equivalent to Penultimate Hop Popping (NEXT) or Ultimate Hop
     * Popping (CONTINUE) in MPLS.
     * pg 10 - Segment Routing Architecture-2*/

    /*supporting fields, Not a TLV part as per the standards*/
     /*conflict resolution : From prefix_sid_subtlv_t , recieving router computes the 
     * sr_mapping_entry_t data structure for all prefixes advertised 
     * with SID. It is not a part of subtlv.*/
} prefix_sid_subtlv_t;


#define IS_PREFIX_SR_ACTIVE(prefixptr)     (prefixptr->conflct_res == SID_ACTIVE)
#define MARK_PREFIX_SR_INACTIVE(prefixptr) (prefixptr->conflct_res = SID_INACTIVE)
#define MARK_PREFIX_SR_ACTIVE(prefixptr)   (prefixptr->conflct_res = SID_ACTIVE)



/*Adjacecncy SID*/

/*If this bit set, this Adj has global scope, else local scope*/
#define ADJ_SID_FLAG_SCOPE  1

/*If unset, Adj is IPV4, else IPV6*/
#define ADJ_SID_ADDRESS_FAMILY_F_FLAG   7

/*If set, the Adj-SID is eligible for
 * protection*/
#define ADJ_SID_BACKUP_B_FLAG   6

/*If set, then the Adj-SID carries a value.
 * By default the flag is SET.*/
#define ADJ_SID_BACKUP_VALUE_V_FLAG 5

/*If set, then the value/index carried by
the Adj-SID has local significance. By default the flag is
SET.*/
#define ADJ_SID_LOCAL_SIGNIFICANCE_L_FLAG   4

/*When set, the S-Flag indicates that the
Adj-SID refers to a set of adjacencies (and therefore MAY be
assigned to other adjacencies as well).*/
#define ADJ_SID_SET_S_FLAG  3

/* When set, the P-Flag indicates that
 * the Adj-SID is persistently allocated, i.e., the Adj-SID value
 * remains consistent across router restart and/or interface flap
 * */
#define ADJ_SID_PERSISTENT_P_FLAG   2


/*All these members are advertised by IGP SR TLVs*/
typedef struct _p2p_adj_sid_subtlv_t{

    BYTE type;      /*Constant = 31*/
    BYTE length;
    BYTE flags;
    BYTE weight;    /*Used for parallel Adjacencies, section 3.4.1,
                      draft-ietf-spring-segment-routing-13 pg 15*/
    /*If local label value is specified (L and V flag sets), then
     * this field contains label value encoded as last 20 bits.
     * if index into srgb is specified, then this field contains
     * is a 4octet value indicating the offset in the SID/Label space
     * advertised bu this router. In this case L and V flag are unset.
     * */
    segment_id_subtlv_t sid;
   /*draft-ietf-spring-segment-routing-13 pg 15*/
} p2p_ajd_sid_subtlv_t;


/*LAN ADJ SID*/
typedef struct _lan_adj_sid_subtlv_t{

    BYTE type;      /*Constant = 32*/
    BYTE length;
    BYTE flags;
    BYTE weight;
    BYTE system_id[6];  /*RFC compliant*/
    //node_t *nbr_node; /*Our implementation compliant*/
    segment_id_subtlv_t sid;
} lan_adg_sid_subtlv_t;


typedef struct _node_t node_t;
typedef struct prefix_ prefix_t;
typedef struct edge_end_ edge_end_t;

/*
If a node N advertises Prefix-SID SID-R for a prefix R that is
attached to N, if N specifies CONTINUE as the operation to be
performed by directly connected neighbors, N MUST maintain the
following FIB entry:
Incoming Active Segment: SID-R
Ingress Operation: NEXT
Egress interface: NULL
    Segment routing Architecture-2, pg11
*/

void
sr_install_local_prefix_mpls_fib_entry(node_t *node, routes_t *route);

/*
A remote node M MUST maintain the following FIB entry for any
learned Prefix-SID SID-R attached to IP prefix R:
Incoming Active Segment: SID-R
Ingress Operation:
If the next-hop of R is the originator of R
and instructed to remove the active segment: NEXT
Else: CONTINUE
Egress interface: the interface towards the next-hop along the
path computed using the algorithm advertised with
the SID toward prefix R.
    Segment routing Architecture-2, pg11
*/

void
sr_install_remote_prefix_mpls_fib_entry(node_t *node, routes_t *route);

/*For ipv6 Data plane*/
/*A node N advertising an IPv6 address R usable as a segment identifier
 * MUST maintain the following FIB entry:
 * Incoming Active Segment: R
 * Ingress Operation: NEXT
 * Egress interface: NULL
 * Segment routing Architecture-2, pg11
 * */

void
sr_install_local_prefix_ipv6_fib_entry(node_t *node, prefix_t *prefix);

/*
Independent of Segment Routing support, any remote IPv6 node will
maintain a plain IPv6 FIB entry for any prefix, no matter if the
represent a segment or not. This allows forwarding of packets to the
node which owns the SID even by nodes which do not support Segment
Routing. */
/*So, this is simple ipv6 unicast forwarding behavior*/
void
sr_install_remote_prefix_ipv6_fib_entry(node_t *node, prefix_t *prefix);

void
allocate_local_adj_sid(node_t *node, edge_end_t *adjacency);

void
allocate_global_adj_sid(node_t *node, edge_end_t *adjacency, srgb_t *srgb);

/*
 *When a node binds an Adj-SID to a local data-link L, the node MUST
 install the following FIB entry:
 Incoming Active Segment: V
 Ingress Operation: NEXT
 Egress Interface: L
 Also take care of section 3.4.1 pg 15
 * */
void
sr_install_local_adj_mpls_fib_entry(node_t *node, edge_end_t *adjacency);

void
sr_install_global_adj_mpls_fib_entry(node_t *node, edge_end_t *adjacency, srgb_t *srgb);

/*
+-------------------------------------------------+
|Conflict Resolution Related APIs                 |
+-------------------------------------------------+
*/

/*srgb ranges should not overlap. If they overlap, the entire SRGB advertised by
 * remote_node is discarded
 * return TRUE if overlap, else return FALSE*/
boolean
is_srgb_ranges_overlap(srgb_t *remote_node_srgb);

#define IGP_DEFAULT_SID_PFX_PREFERENCE_VALUE         192 /*for prefix-SID advertised by IGP running on non-SRMS*/
#define IGP_DEFAULT_SID_SRMS_PFX_PREFERENCE_VALUE    128 /*for prefix-SID advertised by IGP running on SRMS*/

/* This structure is computed by a node for every other level reachable prefix 
 * advertised by remote nodes in the network. This structure is per PFX SID.
 * The mapping entry is defined uniquely by the following tuple : 
 *  (Prf, Pi/L, Si, R, T, A)
 * */
struct _sr_mapping_entry_t{

    unsigned char prf;  /*if 0, SID should be ignored. [0-255] preference value, 
                          default is 192 for any IGP node which do not advertise preference. 
                          Mapping server nodes MAY advertise this preference*/
    unsigned int pi;    /*initial prefix*/
    unsigned int pe;    /*End prefix*/
    char pfx_len;
    char max_pfx_len;   /*32 for IPV4 and 128 for IPV6*/
    unsigned int si;    /*Initial sid value*/
    unsigned int se;    /*End sid value*/
    BYTE range_value;   /*range value , always 1 for IGP*/
    BYTE topology;      /*0 for IPV4, 2 for IPV6*/
    BYTE algorithm;     /*SHORTEST_PATH_FIRST = 0, STRICT_SHORTEST_PATH = 1*/
} ;

/*Fn to construct the mapping entry from prefix SID*/
void
construct_prefix_mapping_entry(prefix_t *prefix, 
            sr_mapping_entry_t *mapping_entry_out);

/* Fn to check if the two prefix conflicts. Two Prefixes p1/m1 and p2/m2 are said to be conflicting 
 * if in sr_mapping_entry_t : topology, algorithm, address-family, and prefix length
 * is equal AND different SID is assigned to same prefix. section 3.2.1 conflict resolution*/
 /*Algorithm to be implemented is defined in  3.2.1.2*/
boolean 
is_prefixes_conflicting(sr_mapping_entry_t *pfx_mapping_entry1, 
    sr_mapping_entry_t *pfx_mapping_entry2);

/*SID conflict*/
boolean
is_prefixes_sid_conflicting(sr_mapping_entry_t *pfx_mapping_entry1,
    sr_mapping_entry_t *pfx_mapping_entry2);

boolean
is_identical_mapping_entries(sr_mapping_entry_t *mapping_entry1,
    sr_mapping_entry_t *mapping_entry2);


/*Test if the route has SPRING path also. If the best prefix
 * prefix originator of the route could not pass conflict-resolution
 * test, then route is said to have no spring path, otherwise it will
 * lead to traffic black-holing
 * */

boolean 
IS_ROUTE_SPRING_CAPABLE(routes_t *route);
        
typedef struct LL ll_t ;

#if 0
ll_t *
prefix_conflict_generic_algorithm(node_t *node, LEVEL level);

ll_t *
prefix_sid_conflict_generic_algorithm(node_t *node, LEVEL level);
#endif

ll_t *
prefix_conflict_resolution(node_t *node, LEVEL level);

ll_t *
prefix_sid_conflict_resolution(ll_t *global_prefix_list);
/*show functions*/

void
resolve_prefix_conflict(prefix_t *prefix1, sr_mapping_entry_t *pfx_mapping_entry1, 
        prefix_t *prefix2, sr_mapping_entry_t *pfx_mapping_entry2);

void
resolve_prefix_sid_conflict(prefix_t *prefix1, sr_mapping_entry_t *pfx_mapping_entry1, 
        prefix_t *prefix2, sr_mapping_entry_t *pfx_mapping_entry2);

void
igp_install_mpls_static_route(node_t *node, char *prefix, char mask);

void
igp_uninstall_mpls_static_route(node_t *node, char *prefix, char mask);


#if 0
void
show_all_prefix_conflicts(node_t *node, LEVEL level);

void
show_all_prefix_sid_conflicts(node_t *node, LEVEL level);
#endif

#endif /* __SR__ */ 
