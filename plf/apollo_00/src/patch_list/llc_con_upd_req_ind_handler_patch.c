#include "patch.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "co_bt.h"
#include "co_llcp.h"
#include "co_error.h"
#include "ke_msg.h"
#include "ke_timer.h"
#include "llm_task.h"
#include "llm_util.h"
#include "llc_llcp.h"
#include "llc_util.h"
#include "llc_task.h"
#include "llc.h"
#include "lld_pdu.h"
#include "lld_util.h"
#include "lld.h"
#include "em_buf.h"

#include "reg_ble_em_rx_desc.h"
#include "reg_ble_em_tx_desc.h"
#include "reg_ble_em_cs.h"

#include "dbg_swdiag.h"

#if (HCI_PRESENT)
#include "hci.h"
#endif //(HCI_PRESENT)
#if (BLE_CHNL_ASSESS)
#include "llc_ch_asses.h"
#if (NVDS_SUPPORT)
#include "nvds.h"
#endif //(NVDS_SUPPORT))
#endif //(BLE_CHNL_ASSESS)

#if(BLE_AUDIO)
#include "reg_blecore.h"
#include "audio.h"
#endif
#if (BLE_PERIPHERAL || BLE_CENTRAL)


//add remote channel_map updata judge
int llc_con_upd_req_ind_handler_patch(ke_msg_id_t const msgid,
                                       struct llc_con_upd_req_ind *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    uint16_t conhdl = KE_IDX_GET(dest_id);
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);


    // check if state is Free or in disconnected state
    if(llc_state_chk(state, LLC_DISC_BUSY))
    {
        // Nothing to do
    }
    // check if local procedure is on-going
    else if(   (llc_state_chk(state, LLC_LOC_PROC_BUSY) && (param != llc_util_get_operation_ptr(conhdl, LLC_OP_LOC_PARAM_UPD)))
            // check if remote connection update not already received
            || (llc_state_chk(state, LLC_REM_PROC_BUSY) && (llc_env[conhdl]->rem_proc_state == LLC_REM_WAIT_CON_UPD_INSTANT))
            // check if remote channel map updata not already received
            || (llc_state_chk(state, LLC_REM_PROC_BUSY) && (llc_env[conhdl]->rem_proc_state == LLC_REM_WAIT_MAP_UPD_INSTANT))
            // check if traffic is paused
            || llc_state_chk(state, LLC_TRAFFIC_PAUSED_BUSY))
    {
        // process this message later
        msg_status = KE_MSG_SAVED;
    }
    else
    {
        struct llc_env_tag *llc_env_ptr = llc_env[conhdl];

        switch(param->operation)
        {
            case LLC_CON_UP_MOVE_ANCHOR:
            {
                /* BLUEX : for multiple connections , do not move anchor point*/
                break;
                /* BLUEX */
            }

            case LLC_CON_UP_HCI_REQ:
            {
                #if !(BLE_QUALIF)
                // forced to no prefered periodicity
                param->pref_period = 0;
                if(!(   GETF(llc_env_ptr->llc_status, LLC_STAT_FEAT_EXCH)
                     && (!(llc_env_ptr->feats_used.feats[0] & BLE_CON_PARAM_REQ_PROC_FEATURE))
                     && (llm_util_check_evt_mask(LE_REM_CON_PARA_REQ_EVT_BIT))))
                {
                    if(param->operation != LLC_CON_UP_MOVE_ANCHOR)
                    {
                        // Connection parameter update is requested by the Host
                        SETF(llc_env_ptr->llc_status, LLC_STAT_UPDATE_HOST_REQ, true);

                        #if (BLE_TESTER)
                        if (llc_env_ptr->tester_params.tester_feats & LLC_TESTER_SURCHARGE_PARAM_REQ)
                        {
                            param->pref_period = llc_env_ptr->tester_params.pref_period;
                            param->offset0     = llc_env_ptr->tester_params.offset0;
                            param->offset1     = llc_env_ptr->tester_params.offset1;
                            param->offset2     = llc_env_ptr->tester_params.offset2;
                            param->offset3     = llc_env_ptr->tester_params.offset3;
                            param->offset4     = llc_env_ptr->tester_params.offset4;
                            param->offset5     = llc_env_ptr->tester_params.offset5;
                        }
                        #endif // (BLE_TESTER)

                        lld_con_param_req(conhdl, llc_env_ptr->elt, param);
                    }

                    llc_llcp_con_param_req_pdu_send(conhdl, param);

                    // Set the state of the LLC
                    llc_state_update(dest_id, &state, LLC_LOC_PROC_BUSY, true);
                    llc_env_ptr->loc_proc_state = LLC_LOC_WAIT_CON_PARAM_RSP;

                    // Start the LLCP Response TO
                    ke_timer_set(LLC_LLCP_RSP_TO, dest_id, LLC_DFT_RSP_TO);

                    llc_util_set_operation_ptr(conhdl, LLC_OP_LOC_PARAM_UPD, (void*)param);

                    msg_status = KE_MSG_NO_FREE;
                    break;
                }
                else
                #endif // !(BLE_QUALIF)
                {
                    // force interval parameter that will be applied during connection update
                    param->interval_min = param->con_intv_min;
                    param->interval_max = param->con_intv_max;
                }
            }
            // no break
            case LLC_CON_UP_FORCE:
            {
                #if (BLE_CENTRAL)
                if (lld_get_mode(conhdl) == LLD_EVT_MST_MODE)
                {
                    // Send connection update request
                    struct llcp_con_upd_ind con_upd_pdu;

                    // Give to the driver the instant parameter and wait the update of the interval parameter
                    lld_con_update_req(llc_env_ptr->elt, param, &con_upd_pdu);

                    // update the environment variable
                    llc_env_ptr->n_sup_to = param->superv_to;

                    // request to the driver to send the pdu
                    llc_llcp_con_update_pdu_send(conhdl, &con_upd_pdu);

                    // store parameter used
                    param->interval_min = con_upd_pdu.interv;
                    param->con_latency  = con_upd_pdu.latency;

                    // Set the state of the LLC
                    llc_state_update(dest_id, &state, LLC_LOC_PROC_BUSY, true);
                    llc_env_ptr->loc_proc_state = LLC_LOC_WAIT_CON_UPD_INSTANT;
                    // Clear Operation message free by the kernel
                    llc_util_set_operation_ptr(conhdl, LLC_OP_LOC_PARAM_UPD, (void*)param);
                    msg_status = KE_MSG_NO_FREE;
                }
                #endif //(BLE_CENTRAL)
            } break;

            #if !(BLE_QUALIF)
            #if (BLE_CENTRAL)
            case LLC_CON_UP_PEER_REQ:
            {
                // allocate the event message
                struct hci_le_rem_con_param_req_evt *event = KE_MSG_ALLOC(HCI_LE_EVENT, conhdl, HCI_LE_META_EVT_CODE, hci_le_rem_con_param_req_evt);

                // fill event parameters
                event->subcode      = HCI_LE_REM_CON_PARAM_REQ_EVT_SUBCODE;
                event->conhdl       = co_htobs(conhdl);
                event->interval_min = param->interval_min;
                event->interval_max = param->interval_max;
                event->latency      = param->con_latency;
                event->timeout      = param->superv_to;

                // send the message
                hci_send_2_host(event);
                //Save the request for future used (when a positive command is sent by the host)
                llc_util_set_operation_ptr(conhdl, LLC_OP_REM_PARAM_UPD, (void*)param);

                llc_env_ptr->rem_proc_state = LLC_REM_WAIT_CON_PARAM_HOST_RSP;
                llc_state_update(dest_id, &state, LLC_REM_PROC_BUSY, true);
                msg_status = KE_MSG_NO_FREE;
            }break;
            case LLC_CON_UP_LOC_REQ:
            {

                // Send connection update request
                struct llcp_con_upd_ind con_upd_pdu;

                 lld_con_update_after_param_req(conhdl, llc_env_ptr->elt, param, &con_upd_pdu, true);

                 /*
                  * Give to the driver the instant parameter and wait the update of the interval
                  * parameter
                  */
                 // update the environment variable
                 llc_env_ptr->n_sup_to = param->superv_to;

                 // request to the driver to send the pdu
                 llc_llcp_con_update_pdu_send(conhdl, &con_upd_pdu);

                 // store parameter used
                 param->interval_min = con_upd_pdu.interv;
                 param->con_latency  = con_upd_pdu.latency;

                 // Set the state of the LLC
                 llc_state_update(dest_id, &state, LLC_LOC_PROC_BUSY, true);
                 llc_env_ptr->loc_proc_state = LLC_LOC_WAIT_CON_UPD_INSTANT;
                 // Clear Operation message free by the kernel
                 llc_util_set_operation_ptr(conhdl, LLC_OP_LOC_PARAM_UPD, (void*)param);
                 msg_status = KE_MSG_NO_FREE;

            } break;
            #endif //(BLE_CENTRAL)
            #endif //#if !(BLE_QUALIF)

            default:
            {
            } break;
        }
    }

    return(msg_status);
}


void llc_con_upd_req_ind_handler_init(void)
{
    uint8_t patch_no[1];
    if(patch_alloc(&patch_no[0])==false)
    {
        BX_ASSERT(0);
    }
    patch_entrance_exit_addr(patch_no[0],0x1ea60,(uint32_t)llc_con_upd_req_ind_handler_patch);
    PATCH_ENABLE(patch_no[0]);
}

#endif

