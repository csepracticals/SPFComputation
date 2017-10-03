/*
 * =====================================================================================
 *
 *       Filename:  spfutil.h
 *
 *    Description:  This file contains utilities for SPFComputation project
 *
 *        Version:  1.0
 *        Created:  Sunday 27 August 2017 01:52:40  IST
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

#ifndef __SPFUTIL__
#define __SPFUTIL__

#include "instance.h"

#define IS_LEVEL_SET(input_level, level)    ((input_level) & (level))
#define SET_LEVEL(input_level, level)       ((input_level) |= (level))


void
copy_nh_list(node_t *src_nh_list[], node_t *dst_nh_list[]);

int
is_nh_list_empty(node_t *nh_list[]);

char *
get_str_level(LEVEL level);

char*
get_str_node_area(AREA area);

void
spf_determine_multi_area_attachment(spf_info_t *spf_info,
                                    node_t *spf_root);

#define THREAD_NODE_TO_STRUCT(structure, member_name, fn_name)                      \
    static inline structure * fn_name (char *member_addr){                          \
        unsigned int member_offset = (unsigned int)&((structure *)0)->member_name;  \
        return (structure *)(member_addr - member_offset);                          \
    }
    
#endif /* __SPFUTIL__ */ 
