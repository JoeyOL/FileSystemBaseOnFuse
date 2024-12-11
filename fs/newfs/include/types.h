#ifndef _TYPES_H_
#define _TYPES_H_

#define MAX_NAME_LEN    128     

/******************************************************************************
* SECTION: Type def
*******************************************************************************/
typedef int          boolean;
typedef uint16_t     flag16;

typedef enum newfs_file_type {
    NEWFS_REG_FILE,
    NEWFS_DIR
    // NEWFS_SYM_LINK
} NEWFS_FILE_TYPE;
/******************************************************************************
* SECTION: Macro
*******************************************************************************/
// 类型定义
#define TRUE                    1
#define FALSE                   0
#define UINT32_BITS             32
#define UINT8_BITS              8

// 磁盘布局相关
#define NEWFS_MAGIC_NUM           0x52415453  
#define NEWFS_SUPER_OFS           0
#define NEWFS_ROOT_INO            0
#define NEWFS_SUPER_BLKS          1
#define NEWFS_INODE_MAP_BLKS      1
#define NEWFS_DATA_MAP_BLKS       1

// 错误类型
#define NEWFS_ERROR_NONE          0
#define NEWFS_ERROR_ACCESS        EACCES
#define NEWFS_ERROR_SEEK          ESPIPE     
#define NEWFS_ERROR_ISDIR         EISDIR
#define NEWFS_ERROR_NOSPACE       ENOSPC
#define NEWFS_ERROR_EXISTS        EEXIST
#define NEWFS_ERROR_NOTFOUND      ENOENT
#define NEWFS_ERROR_UNSUPPORTED   ENXIO
#define NEWFS_ERROR_IO            EIO     /* Error Input/Output */
#define NEWFS_ERROR_INVAL         EINVAL  /* Invalid Args */

// 约束
#define NEWFS_MAX_FILE_NAME       128
#define NEWFS_INODE_PER_FILE      1
#define NEWFS_DATA_PER_FILE       6

// 默认权限（全开）
#define NEWFS_DEFAULT_PERM        0777



/******************************************************************************
* SECTION: Macro Function
*******************************************************************************/
// 磁盘和驱动信息相关
#define NEWFS_IO_SZ()                     (newfs_super.sz_io)
#define NEWFS_BLK_SZ()                    (newfs_super.sz_blk)
#define NEWFS_DISK_SZ()                   (newfs_super.sz_disk)
#define NEWFS_DRIVER()                    (newfs_super.driver_fd)

#define NEWFS_ROUND_DOWN(value, round)    ((value) % (round) == 0 ? (value) : ((value) / (round)) * (round))
#define NEWFS_ROUND_UP(value, round)      ((value) % (round) == 0 ? (value) : ((value) / (round) + 1) * (round))

#define NEWFS_BLKS_SZ(blks)               ((blks) * NEWFS_BLK_SZ())
#define NEWFS_ASSIGN_FNAME(pnewfs_dentry, _fname)\ 
                                        memcpy(pnewfs_dentry->fname, _fname, strlen(_fname))
// 根据索引号求索引偏移
#define NEWFS_INO_OFS(ino)                (newfs_super.inode_offset + ino*sizeof(struct newfs_inode_d))
// 根据数据块号求数据块偏移
#define NEWFS_DATA_OFS(ino)               (newfs_super.data_offset + (ino) * NEWFS_BLKS_SZ(1))
// 文件类型判断
#define NEWFS_IS_DIR(pinode)              (pinode->dentry->ftype == NEWFS_DIR)
#define NEWFS_IS_REG(pinode)              (pinode->dentry->ftype == NEWFS_REG_FILE)

/******************************************************************************
* SECTION: FS Specific Structure - In memory structure
*******************************************************************************/
struct newfs_dentry;
struct newfs_inode;
struct newfs_super;

struct custom_options {
	const char*        device;
	boolean            show_help;
};

struct newfs_inode
{
    int                     ino;                           /* 在inode位图中的下标 */
    // char                    fname[MAX_NAME_LEN];
    int                     size;                          /* 文件已占用空间 */
    int                     dir_cnt;
    struct newfs_dentry*    dentry;                        /* 指向该inode的dentry */
    struct newfs_dentry*    dentrys;                       /* 所有目录项 */
    uint8_t*                data;                           /*数据*/
};  

struct newfs_dentry
{
    char                    fname[MAX_NAME_LEN];
    struct newfs_dentry*    parent;                        /* 父亲Inode的dentry */
    struct newfs_dentry*    brother;                       /* 兄弟 */
    int                     ino;
    struct newfs_inode*     inode;                         /* 指向inode */
    NEWFS_FILE_TYPE         ftype;
};

struct newfs_super
{
    uint32_t           magic; // 幻数
    int                driver_fd; 
    
    int                sz_io; // 512B
    int                sz_blk; // 512<<1
    int                sz_disk; // 4MB
    int                sz_usage; // 已占用空间
    
    int                max_ino; // 最大索引节点数
    uint8_t*           map_inode; // inode位图内存指针
    int                map_inode_blks; 
    int                map_inode_offset;

    int                max_dno; // 最大索引节点数
    uint8_t*           map_data; // data位图内存指针
    int                map_data_blks;  
    int                map_data_offset;

    
    int                inode_offset;
    int                data_offset;
    uint32_t           inode_per_blk;
    uint32_t           inode_blks;

    boolean            is_mounted;
    struct newfs_dentry* root_dentry;
};

static inline struct newfs_dentry* new_dentry(char * fname, NEWFS_FILE_TYPE ftype) {
    struct newfs_dentry * dentry = (struct newfs_dentry *)malloc(sizeof(struct newfs_dentry));
    memset(dentry, 0, sizeof(struct newfs_dentry));
    NEWFS_ASSIGN_FNAME(dentry, fname);
    dentry->ftype   = ftype;
    dentry->ino     = -1;
    dentry->inode   = NULL;
    dentry->parent  = NULL;
    dentry->brother = NULL; 
    return dentry;                                           
}

/******************************************************************************
* SECTION: FS Specific Structure - Disk structure
*******************************************************************************/
struct newfs_super_d
{
    uint32_t           magic_num;
    uint32_t           sz_usage;
    
    uint32_t           max_ino;
    uint32_t           map_inode_blks;
    uint32_t           map_inode_offset;

    uint32_t           max_dno;
    uint32_t           map_data_blks;
    uint32_t           map_data_offset;

    uint32_t           inode_offset;
    uint32_t           data_offset;
    uint32_t           inode_per_blk;
    uint32_t           inode_blks;
};

struct newfs_inode_d
{
    uint32_t           ino;                           /* 在inode位图中的下标 */
    uint32_t           size;                          /* 文件已占用空间 */
    // char               target_path[MAX_NAME_LEN];/* store traget path when it is a symlink */
    uint32_t           dir_cnt;
    NEWFS_FILE_TYPE    ftype;   
    int                blk_pointer[NEWFS_DATA_PER_FILE] /*数据块索引*/
};  

struct newfs_dentry_d
{
    char               fname[MAX_NAME_LEN];
    NEWFS_FILE_TYPE    ftype;
    uint32_t           ino;                           /* 指向的ino号 */
};  


// struct custom_options {
// 	const char*        device;
// };

// struct newfs_super {
//     uint32_t magic;
//     int      fd;
//     /* TODO: Define yourself */
// };

// struct newfs_inode {
//     uint32_t ino;
//     /* TODO: Define yourself */
// };

// struct newfs_dentry {
//     char     name[MAX_NAME_LEN];
//     uint32_t ino;
//     /* TODO: Define yourself */
// };

#endif /* _TYPES_H_ */