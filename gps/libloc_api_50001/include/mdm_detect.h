#ifndef __MDM_DETECT_H__
#define __MDM_DETECT_H__

#define RET_SUCCESS         0
#define MAX_SUPPORTED_MDM   4
#define MAX_NAME_LEN        32
#define MAX_PATH_LEN        255

typedef enum MdmType {
    MDM_TYPE_EXTERNAL,
    MDM_TYPE_INTERNAL,
} MdmType;

struct mdm_info {
    MdmType type;
    char mdm_name[MAX_NAME_LEN];
    char mdm_link[MAX_NAME_LEN];
    char powerup_node[MAX_PATH_LEN];
    char drv_port[MAX_PATH_LEN];
    char ram_dump_path[MAX_PATH_LEN];
    char esoc_node[MAX_NAME_LEN];
};

struct dev_info {
    int num_modems;
    struct mdm_info mdm_list[MAX_SUPPORTED_MDM];
};

int get_system_info(struct dev_info *dev);

#endif
