#ifndef vtk_exodus2_mangle_h
#define vtk_exodus2_mangle_h

/* 

This header file mangles all symbols exported from the exodus library.
It is included in all files while building the exodus library.  Due to
namespace pollution, no exodus headers should be included in .h files in
VTK.

This is the way to recreate the list:

nm bin/libvtkexodIIc.so |grep " [TRD] " | awk '{ print "#define "$3" vtk_exodus_"$3 }' | \
        grep -v vtk_exodus__fini | grep -v vtk_exodus__init | sort

also

#define ex_create vtk_exodus_ex_create 
#define ex_open vtk_exodus_ex_open 

were changed to

#define ex_create_int vtk_exodus_ex_create_int 
#define ex_open_int vtk_exodus_ex_open_int 

after the fact

Note that _fini and _init should be excluded because they are not functions
implemented by the library but are rather created by the linker and
used when the shared library is loaded/unloaded from an executable.

*/

#define ex_dim_num_objects vtk_exodus_ex_dim_num_objects
#define ex_get_attr_param vtk_exodus_ex_get_attr_param
#define ex_get_counter_list vtk_exodus_ex_get_counter_list
#define ex_get_err vtk_exodus_ex_get_err
#define ex_get_id_map vtk_exodus_ex_get_id_map
#define ex_get_stat_ptr vtk_exodus_ex_get_stat_ptr
#define ex_get_truth_table vtk_exodus_ex_get_truth_table
#define ex_get_variable_names vtk_exodus_ex_get_variable_names
#define ex_get_variable_name vtk_exodus_ex_get_variable_name
#define ex_get_variable_param vtk_exodus_ex_get_variable_param
#define ex_inquire_int vtk_exodus_ex_inquire_int
#define ex_name_of_object vtk_exodus_ex_name_of_object
#define ex_put_attr_param vtk_exodus_ex_put_attr_param
#define ex_put_id_map vtk_exodus_ex_put_id_map
#define ex_put_truth_table vtk_exodus_ex_put_truth_table
#define ex_put_variable_names vtk_exodus_ex_put_variable_names
#define ex_put_variable_name vtk_exodus_ex_put_variable_name
#define ex_put_variable_param vtk_exodus_ex_put_variable_param
#define ex_rm_stat_ptr vtk_exodus_ex_rm_stat_ptr
#define ex_var_type_to_ex_entity_type vtk_exodus_ex_var_type_to_ex_entity_type
#define cpy_coord_def vtk_exodus_cpy_coord_def
#define cpy_coord_val vtk_exodus_cpy_coord_val
#define cpy_var_def vtk_exodus_cpy_var_def
#define cpy_var_val vtk_exodus_cpy_var_val
#define dbl_to_flt vtk_exodus_dbl_to_flt
#define ex_catstr2 vtk_exodus_ex_catstr2
#define ex_catstr vtk_exodus_ex_catstr
#define ex_close vtk_exodus_ex_close
#define ex_comp_ws vtk_exodus_ex_comp_ws
#define ex_conv_array vtk_exodus_ex_conv_array
#define ex_conv_exit vtk_exodus_ex_conv_exit
#define ex_conv_ini vtk_exodus_ex_conv_ini
#define ex_copy vtk_exodus_ex_copy
#define ex_create_int vtk_exodus_ex_create_int 
#define ex_cvt_nodes_to_sides vtk_exodus_ex_cvt_nodes_to_sides
#define ex_dim_num_entries_in_object vtk_exodus_ex_dim_num_entries_in_object
#define ex_err vtk_exodus_ex_err
#define ex_get_all_times vtk_exodus_ex_get_all_times
#define ex_get_attr_names vtk_exodus_ex_get_attr_names
#define ex_get_attr vtk_exodus_ex_get_attr
#define ex_get_block vtk_exodus_ex_get_block
#define ex_get_concat_node_sets vtk_exodus_ex_get_concat_node_sets
#define ex_get_concat_sets vtk_exodus_ex_get_concat_sets
#define ex_get_concat_side_sets vtk_exodus_ex_get_concat_side_sets
#define ex_get_conn vtk_exodus_ex_get_conn
#define ex_get_coordinate_frames vtk_exodus_ex_get_coordinate_frames
#define ex_get_coord_names vtk_exodus_ex_get_coord_names
#define ex_get_coord vtk_exodus_ex_get_coord
#define ex_get_cpu_ws vtk_exodus_ex_get_cpu_ws
#define ex_get_dimension vtk_exodus_ex_get_dimension
#define ex_get_elem_attr_names vtk_exodus_ex_get_elem_attr_names
#define ex_get_elem_attr vtk_exodus_ex_get_elem_attr
#define ex_get_elem_blk_ids vtk_exodus_ex_get_elem_blk_ids
#define ex_get_elem_block vtk_exodus_ex_get_elem_block
#define ex_get_elem_conn vtk_exodus_ex_get_elem_conn
#define ex_get_elem_map vtk_exodus_ex_get_elem_map
#define ex_get_elem_num_map vtk_exodus_ex_get_elem_num_map
#define ex_get_elem_varid vtk_exodus_ex_get_elem_varid
#define ex_get_elem_var_tab vtk_exodus_ex_get_elem_var_tab
#define ex_get_elem_var_time vtk_exodus_ex_get_elem_var_time
#define ex_get_elem_var vtk_exodus_ex_get_elem_var
#define ex_get_file_item vtk_exodus_ex_get_file_item
#define ex_get_glob_vars vtk_exodus_ex_get_glob_vars
#define ex_get_glob_var_time vtk_exodus_ex_get_glob_var_time
#define ex_get_ids vtk_exodus_ex_get_ids
#define ex_get_info vtk_exodus_ex_get_info
#define ex_get_init_ext vtk_exodus_ex_get_init_ext
#define ex_get_init vtk_exodus_ex_get_init
#define ex_get_map_param vtk_exodus_ex_get_map_param
#define ex_get_map vtk_exodus_ex_get_map
#define ex_get_names vtk_exodus_ex_get_names
#define ex_get_name vtk_exodus_ex_get_name
#define ex_get_nodal_varid_var vtk_exodus_ex_get_nodal_varid_var
#define ex_get_nodal_varid vtk_exodus_ex_get_nodal_varid
#define ex_get_nodal_var_time vtk_exodus_ex_get_nodal_var_time
#define ex_get_nodal_var vtk_exodus_ex_get_nodal_var
#define ex_get_node_map vtk_exodus_ex_get_node_map
#define ex_get_node_num_map vtk_exodus_ex_get_node_num_map
#define ex_get_node_set_dist_fact vtk_exodus_ex_get_node_set_dist_fact
#define ex_get_node_set_ids vtk_exodus_ex_get_node_set_ids
#define ex_get_node_set_param vtk_exodus_ex_get_node_set_param
#define ex_get_node_set vtk_exodus_ex_get_node_set
#define ex_get_nset_varid vtk_exodus_ex_get_nset_varid
#define ex_get_nset_var_tab vtk_exodus_ex_get_nset_var_tab
#define ex_get_nset_var vtk_exodus_ex_get_nset_var
#define ex_get_num_map vtk_exodus_ex_get_num_map
#define ex_get_num_props vtk_exodus_ex_get_num_props
#define ex_get_object_truth_vector vtk_exodus_ex_get_object_truth_vector
#define ex_get_one_attr vtk_exodus_ex_get_one_attr
#define ex_get_one_elem_attr vtk_exodus_ex_get_one_elem_attr
#define ex_get_partial_elem_map vtk_exodus_ex_get_partial_elem_map
#define ex_get_prop_array vtk_exodus_ex_get_prop_array
#define ex_get_prop_names vtk_exodus_ex_get_prop_names
#define ex_get_prop vtk_exodus_ex_get_prop
#define ex_get_qa vtk_exodus_ex_get_qa
#define ex_get_set_dist_fact vtk_exodus_ex_get_set_dist_fact
#define ex_get_set_param vtk_exodus_ex_get_set_param
#define ex_get_set vtk_exodus_ex_get_set
#define ex_get_side_set_dist_fact vtk_exodus_ex_get_side_set_dist_fact
#define ex_get_side_set_ids vtk_exodus_ex_get_side_set_ids
#define ex_get_side_set_node_count vtk_exodus_ex_get_side_set_node_count
#define ex_get_side_set_node_list_len vtk_exodus_ex_get_side_set_node_list_len
#define ex_get_side_set_node_list vtk_exodus_ex_get_side_set_node_list
#define ex_get_side_set_param vtk_exodus_ex_get_side_set_param
#define ex_get_side_set vtk_exodus_ex_get_side_set
#define ex_get_sset_varid vtk_exodus_ex_get_sset_varid
#define ex_get_sset_var_tab vtk_exodus_ex_get_sset_var_tab
#define ex_get_sset_var vtk_exodus_ex_get_sset_var
#define ex_get_time vtk_exodus_ex_get_time
#define ex_get_varid_var vtk_exodus_ex_get_varid_var
#define ex_get_varid vtk_exodus_ex_get_varid
#define ex_get_var_names vtk_exodus_ex_get_var_names
#define ex_get_var_name vtk_exodus_ex_get_var_name
#define ex_get_var_param vtk_exodus_ex_get_var_param
#define ex_get_var_tab vtk_exodus_ex_get_var_tab
#define ex_get_var_time vtk_exodus_ex_get_var_time
#define ex_get_var vtk_exodus_ex_get_var
#define ex_header_size vtk_exodus_ex_header_size
#define ex_id_lkup vtk_exodus_ex_id_lkup
#define ex_inc_file_item vtk_exodus_ex_inc_file_item
#define ex_inquire vtk_exodus_ex_inquire
#define ex_int_iisort vtk_exodus_ex_int_iisort
#define ex_int_iqsort vtk_exodus_ex_int_iqsort
#define ex_int_median3 vtk_exodus_ex_int_median3
#define ex_iqsort vtk_exodus_ex_iqsort
#define ex_large_model vtk_exodus_ex_large_model
#define ex_name_of_map vtk_exodus_ex_name_of_map
#define ex_name_var_of_object vtk_exodus_ex_name_var_of_object
#define ex_open_int vtk_exodus_ex_open_int 
#define ex_opts vtk_exodus_ex_opts
#define ex_put_all_var_param_ext vtk_exodus_ex_put_all_var_param_ext
#define ex_put_all_var_param vtk_exodus_ex_put_all_var_param
#define ex_put_attr_names vtk_exodus_ex_put_attr_names
#define ex_put_attr vtk_exodus_ex_put_attr
#define ex_put_block vtk_exodus_ex_put_block
#define ex_put_concat_all_blocks vtk_exodus_ex_put_concat_all_blocks
#define ex_put_concat_elem_block vtk_exodus_ex_put_concat_elem_block
#define ex_put_concat_node_sets vtk_exodus_ex_put_concat_node_sets
#define ex_put_concat_sets vtk_exodus_ex_put_concat_sets
#define ex_put_concat_side_sets vtk_exodus_ex_put_concat_side_sets
#define ex_put_concat_var_param vtk_exodus_ex_put_concat_var_param
#define ex_put_conn vtk_exodus_ex_put_conn
#define ex_put_coordinate_frames vtk_exodus_ex_put_coordinate_frames
#define ex_put_coord_names vtk_exodus_ex_put_coord_names
#define ex_put_coord vtk_exodus_ex_put_coord
#define ex_put_elem_attr_names vtk_exodus_ex_put_elem_attr_names
#define ex_put_elem_attr vtk_exodus_ex_put_elem_attr
#define ex_put_elem_block vtk_exodus_ex_put_elem_block
#define ex_put_elem_conn vtk_exodus_ex_put_elem_conn
#define ex_put_elem_map vtk_exodus_ex_put_elem_map
#define ex_put_elem_num_map vtk_exodus_ex_put_elem_num_map
#define ex_put_elem_var_tab vtk_exodus_ex_put_elem_var_tab
#define ex_put_elem_var vtk_exodus_ex_put_elem_var
#define ex_put_glob_vars vtk_exodus_ex_put_glob_vars
#define ex_put_info vtk_exodus_ex_put_info
#define ex_put_init_ext vtk_exodus_ex_put_init_ext
#define ex_put_init vtk_exodus_ex_put_init
#define ex_put_map_param vtk_exodus_ex_put_map_param
#define ex_put_map vtk_exodus_ex_put_map
#define ex_put_names vtk_exodus_ex_put_names
#define ex_put_name vtk_exodus_ex_put_name
#define ex_put_nodal_varid_var vtk_exodus_ex_put_nodal_varid_var
#define ex_put_nodal_var vtk_exodus_ex_put_nodal_var
#define ex_put_node_map vtk_exodus_ex_put_node_map
#define ex_put_node_num_map vtk_exodus_ex_put_node_num_map
#define ex_put_node_set_dist_fact vtk_exodus_ex_put_node_set_dist_fact
#define ex_put_node_set_param vtk_exodus_ex_put_node_set_param
#define ex_put_node_set vtk_exodus_ex_put_node_set
#define ex_put_nset_var_tab vtk_exodus_ex_put_nset_var_tab
#define ex_put_nset_var vtk_exodus_ex_put_nset_var
#define ex_put_num_map vtk_exodus_ex_put_num_map
#define ex_put_one_attr vtk_exodus_ex_put_one_attr
#define ex_put_one_elem_attr vtk_exodus_ex_put_one_elem_attr
#define ex_put_partial_elem_map vtk_exodus_ex_put_partial_elem_map
#define ex_put_prop_array vtk_exodus_ex_put_prop_array
#define ex_put_prop_names vtk_exodus_ex_put_prop_names
#define ex_put_prop vtk_exodus_ex_put_prop
#define ex_put_qa vtk_exodus_ex_put_qa
#define ex_put_set_dist_fact vtk_exodus_ex_put_set_dist_fact
#define ex_put_set_param vtk_exodus_ex_put_set_param
#define ex_put_set vtk_exodus_ex_put_set
#define ex_put_side_set_dist_fact vtk_exodus_ex_put_side_set_dist_fact
#define ex_put_side_set_param vtk_exodus_ex_put_side_set_param
#define ex_put_side_set vtk_exodus_ex_put_side_set
#define ex_put_sset_var_tab vtk_exodus_ex_put_sset_var_tab
#define ex_put_sset_var vtk_exodus_ex_put_sset_var
#define ex_put_time vtk_exodus_ex_put_time
#define ex_put_varid_var vtk_exodus_ex_put_varid_var
#define ex_put_var_names vtk_exodus_ex_put_var_names
#define ex_put_var_name vtk_exodus_ex_put_var_name
#define ex_put_var_param vtk_exodus_ex_put_var_param
#define ex_put_var_tab vtk_exodus_ex_put_var_tab
#define ex_put_var vtk_exodus_ex_put_var
#define ex_rm_file_item vtk_exodus_ex_rm_file_item
#define ex_swap vtk_exodus_ex_swap
#define ex_update vtk_exodus_ex_update
#define flt_to_dbl vtk_exodus_flt_to_dbl
#define get_stat_ptr vtk_exodus_get_stat_ptr
#define itol vtk_exodus_itol
#define ltoi vtk_exodus_ltoi
#define nc_flt_code vtk_exodus_nc_flt_code
#define resize_buffer vtk_exodus_resize_buffer
#define rm_stat_ptr vtk_exodus_rm_stat_ptr
#define update_internal_structs vtk_exodus_update_internal_structs

#endif /* vtk_exodus2_mangle.h */
