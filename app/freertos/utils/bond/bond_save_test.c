/**
 ****************************************************************************************
 *
 * @file   bond_save_test.c
 *
 * @brief  ble bond save test module.
 *
 * @author  Chen Jia Chuang
 * @date    2018-12-26
 * @version <0.0.0.1>
 *
 * @license
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

#include "bond_save.h"
#include "string.h"
#include "log.h"
#include "stdlib.h"
#include "plf.h"
#include "nvds_in_ram.h"
#include "bond_save_flash.h"

/*
 * VARIABLE
 ****************************************************************************************
 */
bond_security_t test_security;
sign_counter_t  test_sign_cnt;
bond_database_t test_database;
bond_cccd_t     test_cccd;

bond_handle_t   test_handle;
bond_node_id_t  node_id;
bond_node_id_t  id_test = BOND_NODE_UNALLOC_ID;
ble_bond_err_t  retval = 0;

uint8_t nvds_usage[256];
uint8_t nvds_usage_buff[256];

/*
 * DECLARE
 ****************************************************************************************
 */
void security_set(uint8_t set_val , uint8_t addr_val , uint8_t addr_type , uint8_t id);



/*
 * TOOLS
 ****************************************************************************************
 */
void add_nodes(uint8_t num)
{
    LOG(3,"add_nodes\n");
    bond_security_info_t info;
    uint8_t read_back_val = 111;
    test_handle.bond_security = &test_security;
    for(uint8_t i=0;i<num;i++)
    {
        //add node
        security_set(i,i,ADDR_RAND  ,BOND_NODE_UNALLOC_ID);
        //read back
        memset(&info , 0 , sizeof(info));
        bond_save_security_get(id_test , BONDSAVE_SECURITY_LTK_LOCAL  , &info.ltk_local);
        bond_save_security_get(id_test , BONDSAVE_SECURITY_LTK_PEER   , &info.ltk_peer);
        bond_save_security_get(id_test , BONDSAVE_SECURITY_CSRK_LOCAL , &info.csrk_local);
        bond_save_security_get(id_test , BONDSAVE_SECURITY_CSRK_PEER  , &info.csrk_peer);
        bond_save_security_get(id_test , BONDSAVE_SECURITY_IRK        , &info.irk);
        bond_save_security_get(id_test , BONDSAVE_SECURITY_AUTH       , &info.auth);
        read_back_val = memcmp(&info , &test_security.info , sizeof(info)-1);//not align
        LOG(3,"read_back_val = %d\n",read_back_val);
    }
}

void check_nvds_usage(void)
{
    uint8_t nvds_err;
    uint8_t free_val=0;
    nvds_tag_len_t length;
    for(uint8_t i=1;i<255;i++)
    {
        length = 255;
        nvds_err = nvds_get(i , &length , (void*)nvds_usage_buff);
        if(nvds_err == NVDS_OK)
        {
            nvds_usage[i] = 1;
        }
        else
        {
            nvds_usage[i] = 0;
            free_val ++;
        }
    }
    LOG(3,"check_nvds_usage free=%d\n",free_val);
}




/*
 * SECURITY TEST
 ****************************************************************************************
 */
void recover_cb(bool success , bond_node_id_t id)
{
    LOG(3,"success=%d,id=%d\n",success,id);
    node_id = id;
}

void security_set(uint8_t set_val , uint8_t addr_val , uint8_t addr_type , uint8_t id)
{
    memset(&test_security , set_val , sizeof(test_security));
    memset(test_security.bdaddr.addr.addr , addr_val , 6);
    test_security.bdaddr.addr_type = addr_type;
    test_security.info.ltk_present  = 1;
    test_security.info.csrk_present = 1;
    test_security.info.irk_present  = 1;
    if(id == BOND_NODE_UNALLOC_ID)
    {
        bond_save_allocate_new_id(&id_test);
    }
    retval = bond_save_param_set(id_test , BOND_SAVE_TYPE_SECURITY , test_handle);
    LOG(3,"retval=%d,id_test=%d\n",retval,id_test);
}

void security_recover(uint8_t set_val , uint8_t addr_val , uint8_t addr_type , uint8_t role)
{
    memset(&test_security , set_val , sizeof(test_security));
    memset(test_security.bdaddr.addr.addr , addr_val , 6);
    test_security.bdaddr.addr_type = addr_type;
    bond_save_recover_security(&test_security.bdaddr , recover_cb);
}

void bond_save_test_recover_basic_test(void)
{
    test_handle.bond_security = &test_security;
    
    //test recover with address (empty nvds)
    security_recover(0x00,0x01,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );

    security_set(0xA0,0xA0,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 0
    security_set(0xA1,0xA1,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 1
    security_set(0xA2,0xA2,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 2
    security_set(0xA3,0xA3,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 3
    
    security_set(0xA4,0xA4,ADDR_RAND  ,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 4
    security_set(0xA5,0xA5,ADDR_RAND  ,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 5
    security_set(0xA6,0xA6,ADDR_RAND  ,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 6
    security_set(0xA7,0xA7,ADDR_RAND  ,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 7
    
    security_set(0x88,0x88,ADDR_PUBLIC,0x02);//save with exist id(2)
    security_set(0xCC,0xCC,ADDR_PUBLIC,0x03);//save with exist id(3)
    
    //try to recover with 0xA1 adderss 
    security_recover(0x00,0xA1,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );    
    //===bkpt===
    
    //try to recover with 0xA1 adderss 
    security_recover(0x00,0xA7,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );    
    //===bkpt===
    
    //try to read 0x02 node
    id_test = 0x02;
    retval = bond_save_param_get(id_test , BOND_SAVE_TYPE_SECURITY , test_handle);  
    LOG(3,"retval=%d,id_test=%d\n",retval,id_test);
    //===bkpt===
}

void different_role_same_addr_test(void)
{
    test_handle.bond_security = &test_security;
    //add node
    security_set(0x11,0xA0,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 0
    security_set(0x22,0xA0,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 1
    security_set(0x33,0xA1,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 2
    security_set(0x44,0xA2,ADDR_PUBLIC,BOND_NODE_UNALLOC_ID);//save an address with unalloc id - 3
    //recover test
    security_recover(0x00,0xA0,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );    
    security_recover(0x00,0xA0,ADDR_PUBLIC,GAP_ROLE_PERIPHERAL);    
    security_recover(0x00,0xA1,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );    
    security_recover(0x00,0xA1,ADDR_PUBLIC,GAP_ROLE_PERIPHERAL);    
    security_recover(0x00,0xA2,ADDR_PUBLIC,GAP_ROLE_CENTRAL   );    
    security_recover(0x00,0xA2,ADDR_PUBLIC,GAP_ROLE_PERIPHERAL);    
}


/*
 * AGE  DELETE TEST
 ****************************************************************************************
 */
void security_age_delete_test(void)
{
    uint8_t i=0;
    
    test_handle.bond_security = &test_security;
    
    #if 1
    for(i=0;i<100;i++)
    {
        security_set(i,i,ADDR_RAND ,BOND_NODE_UNALLOC_ID);
        //===bkpt===
    }
    #endif
    
    #if 0
    //delete spec node
    for(i=0;i<10;i++)
    {
        retval = bond_save_delete_node(i);
        LOG(3,"delete_node retval=%d\n",retval);
    }
    #endif
    
    #if 0
    //delete all
    retval = bond_save_delete_all();
    LOG(3,"delete_all retval=%d\n",retval);
    #endif
}



/*
 * SIGN COUNTER TEST
 ****************************************************************************************
 */
#define TEST_SN_LOOP    10
void sign_counter_test(uint8_t num)
{
    add_nodes(num);// add node
    
    uint8_t i,loop,err;
    test_handle.sign_counter = &test_sign_cnt;
    LOG(3,"=======================================SIGN_COUNTER===loop=%d\n",TEST_SN_LOOP);
    for(loop=1;loop<=TEST_SN_LOOP;loop++)
    {
        LOG(3,"LOOP=%d\n",loop);
        //set
        for(i=0;i<=9;i++)
        {
            node_id = i;
            test_sign_cnt.local = i+100*loop;
            test_sign_cnt.peer  = i * 10*loop;
            retval = bond_save_param_set(node_id , BOND_SAVE_TYPE_SIGN_COUNTER , test_handle);
            BX_DELAY_US(1*1000);
        }
        //get
        for(i=0;i<=9;i++)
        {
            node_id = i;
            retval = bond_save_param_get(node_id , BOND_SAVE_TYPE_SIGN_COUNTER , test_handle);
            err = ((test_sign_cnt.local == i+100*loop) && (test_sign_cnt.peer  == i * 10*loop)) ? 0 : 1;
            if(err) 
            {
                LOG(3,"\t\t\t\t=====get:retval=%d,id=%d,local=%d,peer=%d,loop=%d\n",retval,node_id,test_sign_cnt.local,test_sign_cnt.peer,loop);
            }
            BX_DELAY_US(1*1000);
        }
    }
}

/*
 * CCCD TEST
 ****************************************************************************************
 */
#define TEST_CD_NODES   10
bond_cccd_t test_cccds[TEST_CD_NODES][BOND_SAVE_MAX_CCCD_CNT];
void cccd_test(uint8_t num)
{
    add_nodes(num);// add node
    
    uint8_t node,i;
    test_handle.bond_cccd = &test_cccd;
    node_id = 3;
    uint8_t err=0;
    LOG(3,"=======================================CCCD===loop=%d\n",TEST_CD_NODES);
    
    //set
    for(node = 0;node<TEST_CD_NODES;node++)
    {
        LOG(3,"SET:node=%d\n",node);
        //set once
        for(i=0;i<BOND_SAVE_MAX_CCCD_CNT;i++)
        {
            test_cccds[node][i].attr_handle = i;
            test_cccds[node][i].value       = rand();
            test_cccd = test_cccds[node][i];
            retval = bond_save_param_set(node , BOND_SAVE_TYPE_CCCD , test_handle);
            BX_DELAY_US(1*1000);
        }
        //set again override
        for(i=0;i<BOND_SAVE_MAX_CCCD_CNT;i++)
        {
            test_cccds[node][i].attr_handle = i;
            test_cccds[node][i].value       = rand();
            test_cccd = test_cccds[node][i];
            retval = bond_save_param_set(node , BOND_SAVE_TYPE_CCCD , test_handle);
            BX_DELAY_US(1*1000);
        }
    }
    //get
    for(node = 0;node<TEST_CD_NODES;node++)
    {
        LOG(3,"GET:node=%d\n",node);
        //get
        for(i=0;i<BOND_SAVE_MAX_CCCD_CNT;i++)
        {
            test_cccd.attr_handle = test_cccds[node][i].attr_handle;
            retval = bond_save_param_get(node , BOND_SAVE_TYPE_CCCD , test_handle);
            err = (test_cccd.value == test_cccds[node][i].value) ? 0 : 1;
            if(err) 
            {
                LOG(3,"\t\t\t\t=====get:retval=%d,hdl=%d,val=%d\n",retval,test_cccd.attr_handle,test_cccd.value);
            }
            BX_DELAY_US(1*1000);
        }
    }
    //debug ram content
    bond_save_param_get(node_id , BOND_SAVE_TYPE_CCCD , test_handle);
}

/*
 * DATABASE TEST
 ****************************************************************************************
 */
#define TEST_SERVS_PER_NODE 10  //max=10
#define TEST_SERVS  48  //max=48
#define TEST_LENGTH 100
#define TEST_DB_LOOP    20

typedef struct
{
    uint8_t db[TEST_LENGTH];
    uint8_t length;
    uint8_t uuid[16];
}my_db_t;

my_db_t test_db[TEST_SERVS];
uint8_t test_read_db[TEST_LENGTH];


//nvds same uuid multi test
void database_test(uint8_t num)
{
    add_nodes(num);// add node
    
    uint8_t j,i,err,loop;
    test_handle.bond_database = &test_database;
    node_id = 3;
    LOG(3,"=======================================DATABASE===loop=%d,services_num=%d,lengh=rand\n",TEST_DB_LOOP,TEST_SERVS);
    //init uuid
    for(i=0;i<TEST_SERVS;i++)
    {
        for(j=0;j<16;j++)  {test_db[i].uuid[j] = rand();}
    }
    for(loop=1;loop<=TEST_DB_LOOP;loop++)
    {
        LOG(3,"LOOP=%d\n",loop);
        //set random data
        for(i=0;i<TEST_SERVS;i++)
        {
            test_db[i].length = (rand()%TEST_LENGTH)+1;
            for(j=0;j<TEST_LENGTH;j++)  {test_db[i].db[j] = rand();}
        }
        //set database
        for(i=0;i<TEST_SERVS;i++)
        {
            //node
            node_id = (i/TEST_SERVS_PER_NODE);
            //data
            test_database.length    = test_db[i].length;
            test_database.char_buff = test_db[i].db;
            memcpy(test_database.uuid , test_db[i].uuid , 16);
            //write
            retval = bond_save_param_set(node_id , BOND_SAVE_TYPE_DATABASE , test_handle);
            BX_DELAY_US(1*1000);
        }
        //read database
        for(i=0;i<TEST_SERVS;i++)
        {
            //node
            node_id = (i/TEST_SERVS_PER_NODE);
            //data
            test_database.length    = test_db[i].length;
            test_database.char_buff = test_read_db;
            memcpy(test_database.uuid , test_db[i].uuid , 16);
            //write
            retval = bond_save_param_get(node_id , BOND_SAVE_TYPE_DATABASE , test_handle);
            err = memcmp(test_read_db , test_db[i].db , test_db[i].length);
            if(err) 
            {
                LOG(3,"ERROR!!!!!!!get:retval=%d , loop=%d , id=%d , err=%d\n",retval,loop,i,err);
            }
            BX_DELAY_US(1*1000);
        }
    }
    //for debug
    bond_save_param_get( node_id , BOND_SAVE_TYPE_DATABASE , test_handle);
}


#define UUID_NODES  4
#define UUID_NUM    BOND_SAVE_MAX_DATABASE_CNT  //10
my_db_t test_db_diff[UUID_NODES][UUID_NUM];

//nvds differen multi uuid in one node save test
void database_test_diff_uuid_in_a_node(uint8_t num)
{
    add_nodes(num);// add node
    
    uint8_t j,uuid_idx,err,node;
    test_handle.bond_database = &test_database;
    LOG(3,"=======================================DATABASE MULTI UUID IN ONE NODE===node=%d,UUID_NUM=%d,lengh=rand\n",UUID_NODES,UUID_NUM);

    //write data
    for(node=0;node<UUID_NODES;node++)//node
    {
        for(uuid_idx=0;uuid_idx<UUID_NUM;uuid_idx++)//uuids
        {
            //init uuid
            for(j=0;j<16;j++)  {test_db_diff[node][uuid_idx].uuid[j] = rand();}
            //set random data
            test_db_diff[node][uuid_idx].length = (rand()%TEST_LENGTH)+1;
            for(j=0;j<TEST_LENGTH;j++)  {test_db_diff[node][uuid_idx].db[j] = rand();}
        }
    }
    //set data
    for(node=0;node<UUID_NODES;node++)//node
    {
        for(uuid_idx=0;uuid_idx<UUID_NUM;uuid_idx++)//uuids
        {
            //data
            test_database.length    = test_db_diff[node][uuid_idx].length;
            test_database.char_buff = test_db_diff[node][uuid_idx].db;
            memcpy(test_database.uuid , test_db_diff[node][uuid_idx].uuid , 16);
            //write
            retval = bond_save_param_set(node , BOND_SAVE_TYPE_DATABASE , test_handle);
            //LOG(3,"retval=%d\n",retval);
            BX_DELAY_US(1*1000);
        }
    }
    //get data
    for(node=0;node<UUID_NODES;node++)//node
    {
        for(uuid_idx=0;uuid_idx<UUID_NUM;uuid_idx++)//uuids
        {
            LOG(3,"DB_DIFF_node=%d,uid=%d,",node,uuid_idx);
            //data
            test_database.length    = test_db_diff[node][uuid_idx].length;
            test_database.char_buff = test_read_db;
            memcpy(test_database.uuid , test_db_diff[node][uuid_idx].uuid , 16);
            //write
            retval = bond_save_param_get(node , BOND_SAVE_TYPE_DATABASE , test_handle);
            LOG(3,"retval=%d\n",retval);
            err = memcmp(test_read_db , test_db_diff[node][uuid_idx].db , test_db_diff[node][uuid_idx].length);
            if(err) 
            {
                LOG(3,"ERROR!!!!!!!get:retval=%d , node=%d , uuid_idx=%d , err=%d\n",retval,node,uuid_idx,err);
            }
            BX_DELAY_US(1*1000);
        }
    }
}

/*
 * ALL TEST
 ****************************************************************************************
 */
void all_test(void)
{
    check_nvds_usage();
    
    database_test(10);
    database_test_diff_uuid_in_a_node(10);
    cccd_test(0);
    sign_counter_test(0); 
    
    check_nvds_usage();
    
    #if 0
    //delete spec
    uint8_t i=3;
    while(i --> 0)
    {
        retval = bond_save_delete_node(i);
        LOG(3,"delete_node retval=%d\n",retval);
    }
    
    check_nvds_usage();
    #endif
    
    //delete all
    retval = bond_save_delete_all();
    LOG(3,"delete_all retval=%d\n",retval);
    
    check_nvds_usage();
    LOG(3,"aa aa\n");
}

/*
 * DEBUG TEST
 ****************************************************************************************
 */
#define NVDS_TEST_TAG   0xF0
#define NVDS_TEST_LEN   200
#define NVDS_TEST_TOTAL (200*1000)

uint8_t nvds_test_buff_set[NVDS_TEST_LEN];
uint8_t nvds_test_buff_get[NVDS_TEST_LEN];
const uint32_t test_all_num = NVDS_TEST_TOTAL / NVDS_TEST_LEN;

typedef struct
{
    uint8_t c1 : 1;
    uint8_t c2 : 4;
    uint8_t c3 : 5;
    uint8_t c4 : 6;
    uint8_t c5 : 1;
    
}test_ss;

void nvds_override_test()
{
    uint32_t i,j;
    uint8_t nvds_err;
    nvds_tag_len_t length = NVDS_TEST_LEN;
    nvds_test_buff_set[0]=sizeof(test_ss);
    check_nvds_usage();
    
    for(i=0;i<test_all_num;i++)
    {
        //fill random
        for(j=0;j<NVDS_TEST_LEN;j++) j[nvds_test_buff_set]=rand();
        LOG(3,"idx=%d\t",i);
        //set
        length = NVDS_TEST_LEN;
        nvds_err = nvds_put(NVDS_TEST_TAG, length , (void*)nvds_test_buff_set);
        LOG(3,"put=%d\t",nvds_err);
        //get
        length = NVDS_TEST_LEN;
        nvds_err = nvds_get(NVDS_TEST_TAG , &length , (void*)nvds_test_buff_get);
        LOG(3,"get=%d\t",nvds_err);
        //compare
        nvds_err = memcmp(nvds_test_buff_set , nvds_test_buff_get , NVDS_TEST_LEN);
        LOG(3,"cmp=%d\n",nvds_err);
    }
    
    check_nvds_usage();
    
    LOG(3,"TEST OVER!!\n");
}


/*
 * NVDS USAGE TEST
 ****************************************************************************************
 */
uint32_t nvds_node_decurity , nvds_node_signcnt , nvds_node_database , nvds_node_cccd , nvds_dat_per_node;
uint32_t db_data_usage , node_data_usage , nvds_all_usage;

void debug_nvds_usage_test(void)
{
    nvds_node_decurity = NVDS_LEN_BOND_SAVE_SECURITY;
    nvds_node_signcnt  = NVDS_LEN_BOND_SAVE_SIGN_COUNTER;
    nvds_node_database = NVDS_LEN_BOND_SAVE_DATABASE_IDX;
    nvds_node_cccd     = NVDS_LEN_BOND_SAVE_CCCD;
    nvds_dat_per_node  = nvds_node_decurity + nvds_node_signcnt + nvds_node_database + nvds_node_cccd + 3*4;
    
    db_data_usage      = nvds_dat_per_node * 12;
    node_data_usage    = 0x00FF * BOND_SAVE_MAX_DB_SIZE;
    nvds_all_usage     = db_data_usage + node_data_usage;
    
    //ALL USAGE = 0x3FFC
    LOG(3,"ALL USAGE = 0x%x\n",nvds_all_usage);
}



/*
 * DEBUG TEST
 ****************************************************************************************
 */
typedef enum
{
    TE_1 = (uint32_t)0x77441100,
    TE_2 = (uint32_t)0x77663355,
    TE_3 = (uint32_t)0x44112233,
} te_t;

typedef enum
{
    TB_1 = (uint32_t)0x77441100,
    TB_2 = (uint32_t)0x77663355,
    TB_3 = (uint32_t)0x44112233,
} tb_t;

void debug_test(volatile int a)
{
    volatile te_t aaa=TE_1;
    volatile tb_t bbb=TB_1;
    
    //uint8_t arr[100]={1,[50]=1,};
    LOG(3,"%d\n",aaa,bbb);
}

void bond_save_test_main(void)
{
    bond_save_init();
    //debug_nvds_usage_test();
    //nvds_override_test();
    //bond_save_test_recover_basic_test();
    //different_role_same_addr_test();
    //security_age_delete_test();
    //sign_counter_test(10);
    //cccd_test(10);
    //database_test(10);
    
    
    all_test();
    debug_test(11);
    while((*(__IO uint32_t*)4) != 0)
    {
        ;
    }
}

#if 0
PASS:
different role in same address recover test:public address
bond_save_init
device_age/new_device/delete_node/delete_all
recover -> public_addr/random_addr , override, (without IRK decrypt)
set/get -> security/sign counter/cccd/databaes    : all override,over max range test pass
delete with all data
DATABASE:
out range exception catch test:TEST_SERVS_PER_NODE¡¢TEST_SERVS 
test save less database buff
test bond_save_security_get api

TODO:



FAIL:




#endif





