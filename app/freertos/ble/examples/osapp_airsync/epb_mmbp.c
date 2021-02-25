#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "epb_mmbp.h"
#include "osapp_config.h"
#include "osapp_svc_manager.h"
#include <stdlib.h>
#include "osapp_utils.h"
#include "bx_log.h"


#define TAG_BaseResponse_ErrCode                     0x08
#define TAG_BaseResponse_ErrMsg                      0x12

#define TAG_auth_request_base_request                0x0a
#define TAG_auth_request_Md5DeviceTypeAndDeviceId    0x12
#define TAG_auth_request_ProtoVersion                0x18
#define TAG_auth_request_AuthProto                   0x20
#define TAG_auth_request_AuthMethod                  0x28
#define TAG_auth_request_AesSign                     0x32
#define TAG_auth_request_MacAddress                  0x3a
#define TAG_auth_request_TimeZone                    0x52
#define TAG_auth_request_Language                    0x5a
#define TAG_auth_request_DeviceName                  0x62

#define TAG_auth_response_BaseResponse               0x0a
#define TAG_auth_response_AesSessionKey              0x12

#define TAG_init_request_base_request                0x0a
#define TAG_init_request_RespFieldFilter             0x12
#define TAG_init_request_Challenge                   0x1a

#define TAG_init_response_BaseResponse               0x0a
#define TAG_init_response_UserIdHigh                 0x10
#define TAG_init_response_UserIdLow                  0x18
#define TAG_init_response_ChalleangeAnswer           0x20
#define TAG_init_response_InitScence                 0x28
#define TAG_init_response_AutoSyncMaxDurationSecond  0x30
#define TAG_init_response_UserNickName               0x5a
#define TAG_init_response_PlatformType               0x60
#define TAG_init_response_Model                      0x6a
#define TAG_init_response_Os                         0x72
#define TAG_init_response_Time                       0x78
#define TAG_init_response_TimeZone                   0x8001
#define TAG_init_response_TimeString                 0x8a01

#define TAG_send_data_request_base_request           0x0a
#define TAG_send_data_request_Data                   0x12
#define TAG_send_data_request_Type                   0x18

#define TAG_send_data_response_BaseResponse          0x0a
#define TAG_send_data_response_Data                  0x12

#define TAG_recv_data_push_BasePush                  0x0a
#define TAG_recv_data_push_Data                      0x12
#define TAG_recv_data_push_Type                      0x18

#define TAG_switch_switch_view_BasePush              0x0a
#define TAG_switch_switch_view_SwitchViewOp          0x10
#define TAG_switch_switch_view_ViewId                0x18

#define TAG_switch_backgroud_push_BasePush            0x0a
#define TAG_switch_backgroud_push_SwitchBackgroundOp  0x10


int epb_base_request_pack_size(base_request *request)
{
    int pack_size = 0;
    return pack_size;
}

int epb_pack_base_request(base_request *request, uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_pack_init(&epb, buf, buf_len);
    return epb_get_packed_size(&epb);
}

BaseResponse *epb_unpack_base_response(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    if (!epb_has_tag(&epb, TAG_BaseResponse_ErrCode)) {
        return NULL;
    }
    BaseResponse *response = (BaseResponse *)pvPortMalloc (sizeof(BaseResponse));
    memset(response, 0, sizeof(BaseResponse));
    response->err_code = epb_get_int32(&epb, TAG_BaseResponse_ErrCode);
    if (epb_has_tag(&epb, TAG_BaseResponse_ErrMsg)) {
        response->err_msg.str = epb_get_string(&epb, TAG_BaseResponse_ErrMsg, &response->err_msg.len);
        response->has_err_msg = true;
    }
    return response;
}

void epb_unpack_base_response_free(BaseResponse *response)
{
    vPortFree(response);
}

BasePush *epb_unpack_base_push(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    BasePush *push = (BasePush *)pvPortMalloc (sizeof(BasePush));
    memset(push, 0, sizeof(BasePush));
    return push;
}

void epb_unpack_base_push_free(BasePush *push)
{
    vPortFree(push);
}

int epb_auth_request_pack_size(auth_request *request)
{
    int pack_size = 0;
    int len = 0;

    len = epb_base_request_pack_size(request->base_request);
    pack_size += epb_length_delimited_pack_size(TAG_auth_request_base_request, len);
    if (request->has_md5_device_type_and_device_id) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_Md5DeviceTypeAndDeviceId, request->md5_device_type_and_device_id.len);
    }
    pack_size += epb_varint32_pack_size(TAG_auth_request_ProtoVersion, request->proto_version, false);
    pack_size += epb_varint32_pack_size(TAG_auth_request_AuthProto, request->auth_proto, false);
    pack_size += epb_varint32_pack_size(TAG_auth_request_AuthMethod, request->auth_method, false);
    if (request->has_aes_sign) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_AesSign, request->aes_sign.len);
    }
    if (request->has_mac_address) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_MacAddress, request->mac_address.len);
    }
    if (request->has_time_zone) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_TimeZone, request->time_zone.len);
    }
    if (request->has_language) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_Language, request->language.len);
    }
    if (request->has_device_name) {
        pack_size += epb_length_delimited_pack_size(TAG_auth_request_DeviceName, request->device_name.len);
    }
    return pack_size;
}

int epb_pack_auth_request(auth_request *request, uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_pack_init(&epb, buf, buf_len);
    int ret;
    int tmp_len;
    uint8_t *tmp;
    tmp_len = epb_base_request_pack_size(request->base_request);
    tmp = NULL;
    ret = epb_pack_base_request(request->base_request, tmp, tmp_len);
    if (ret < 0) {
        return ret;
    }
    ret = epb_set_message(&epb, TAG_auth_request_base_request, tmp, tmp_len);
    if (ret < 0) return ret;
    if (request->has_md5_device_type_and_device_id) {
        ret = epb_set_bytes(&epb, TAG_auth_request_Md5DeviceTypeAndDeviceId, request->md5_device_type_and_device_id.data, request->md5_device_type_and_device_id.len);
        if (ret < 0) return ret;
    }
    ret = epb_set_int32(&epb, TAG_auth_request_ProtoVersion, request->proto_version);
    if (ret < 0) return ret;
    ret = epb_set_int32(&epb, TAG_auth_request_AuthProto, request->auth_proto);
    if (ret < 0) return ret;
    ret = epb_set_enum(&epb, TAG_auth_request_AuthMethod, request->auth_method);
    if (ret < 0) return ret;
    if (request->has_aes_sign) {
        ret = epb_set_bytes(&epb, TAG_auth_request_AesSign, request->aes_sign.data, request->aes_sign.len);
        if (ret < 0) return ret;
    }
    if (request->has_mac_address) {
        ret = epb_set_bytes(&epb, TAG_auth_request_MacAddress, request->mac_address.data, request->mac_address.len);
        if (ret < 0) return ret;
    }
    if (request->has_time_zone) {
        ret = epb_set_string(&epb, TAG_auth_request_TimeZone, request->time_zone.str, request->time_zone.len);
        if (ret < 0) return ret;
    }
    if (request->has_language) {
        ret = epb_set_string(&epb, TAG_auth_request_Language, request->language.str, request->language.len);
        if (ret < 0) return ret;
    }
    if (request->has_device_name) {
        ret = epb_set_string(&epb, TAG_auth_request_DeviceName, request->device_name.str, request->device_name.len);
        if (ret < 0) return ret;
    }

    return epb_get_packed_size(&epb);
}

auth_response *epb_unpack_auth_response(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_auth_response_BaseResponse)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_auth_response_AesSessionKey)) {
        return NULL;
    }

    auth_response *response = (auth_response *)pvPortMalloc(sizeof(auth_response));
    memset(response, 0, sizeof(auth_response));
    tmp = epb_get_message(&epb, TAG_auth_response_BaseResponse, &tmp_len);
    response->base_response = epb_unpack_base_response(tmp, tmp_len);
    if (response->base_response == NULL) {
        vPortFree(response);
        return NULL;
    }
    response->aes_session_key.data = epb_get_bytes(&epb, TAG_auth_response_AesSessionKey, &response->aes_session_key.len);
    return response;
}

void epb_unpack_auth_response_free(auth_response *response)
{
    epb_unpack_base_response_free(response->base_response);
    vPortFree(response);
}

int epb_init_request_pack_size(init_request *request)
{
    int pack_size = 0;
    int len = 0;
    len = epb_base_request_pack_size(request->base_request);
    pack_size += epb_length_delimited_pack_size(TAG_init_request_base_request, len);
    if (request->has_resp_field_filter) {
        pack_size += epb_length_delimited_pack_size(TAG_init_request_RespFieldFilter, request->resp_field_filter.len);
    }
    if (request->has_challenge) {
        pack_size += epb_length_delimited_pack_size(TAG_init_request_Challenge, request->challenge.len);
    }
    return pack_size;
}

int epb_pack_init_request(init_request *request, uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_pack_init(&epb, buf, buf_len);
    int ret;
    int tmp_len;
    uint8_t *tmp;
    tmp_len = epb_base_request_pack_size(request->base_request);
    tmp = NULL;
    ret = epb_pack_base_request(request->base_request, tmp, tmp_len);
    if (ret < 0) {
        return ret;
    }
    ret = epb_set_message(&epb, TAG_init_request_base_request, tmp, tmp_len);
    if (ret < 0) return ret;
    if (request->has_resp_field_filter) {
        ret = epb_set_bytes(&epb, TAG_init_request_RespFieldFilter, request->resp_field_filter.data, request->resp_field_filter.len);
        if (ret < 0) return ret;
    }
    if (request->has_challenge) {
        ret = epb_set_bytes(&epb, TAG_init_request_Challenge, request->challenge.data, request->challenge.len);
        if (ret < 0) return ret;
    }
    return epb_get_packed_size(&epb);
}

init_response *epb_unpack_init_response(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_init_response_BaseResponse)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_init_response_UserIdHigh)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_init_response_UserIdLow)) {
        return NULL;
    }
    init_response *response = (init_response *)pvPortMalloc (sizeof(init_response));
    memset(response, 0, sizeof(init_response));
    tmp = epb_get_message(&epb, TAG_init_response_BaseResponse, &tmp_len);
    response->base_response = epb_unpack_base_response(tmp, tmp_len);
    if (response->base_response == NULL) {
        vPortFree(response);
        return NULL;
    }
    response->user_id_high = epb_get_uint32(&epb, TAG_init_response_UserIdHigh);
    response->user_id_low = epb_get_uint32(&epb, TAG_init_response_UserIdLow);
    if (epb_has_tag(&epb, TAG_init_response_ChalleangeAnswer)) {
        response->challeange_answer = epb_get_uint32(&epb, TAG_init_response_ChalleangeAnswer);
        response->has_challeange_answer = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_InitScence)) {
        response->init_scence = (EmInitScence)epb_get_enum(&epb, TAG_init_response_InitScence);
        response->has_init_scence = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_AutoSyncMaxDurationSecond)) {
        response->auto_sync_max_duration_second = epb_get_uint32(&epb, TAG_init_response_AutoSyncMaxDurationSecond);
        response->has_auto_sync_max_duration_second = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_UserNickName)) {
        response->user_nick_name.str = epb_get_string(&epb, TAG_init_response_UserNickName, &response->user_nick_name.len);
        response->has_user_nick_name = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_PlatformType)) {
        response->platform_type = (EmPlatformType)epb_get_enum(&epb, TAG_init_response_PlatformType);
        response->has_platform_type = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_Model)) {
        response->model.str = epb_get_string(&epb, TAG_init_response_Model, &response->model.len);
        response->has_model = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_Os)) {
        response->os.str = epb_get_string(&epb, TAG_init_response_Os, &response->os.len);
        response->has_os = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_Time)) {
        response->time = epb_get_int32(&epb, TAG_init_response_Time);
        response->has_time = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_TimeZone)) {
        response->time_zone = epb_get_int32(&epb, TAG_init_response_TimeZone);
        response->has_time_zone = true;
    }
    if (epb_has_tag(&epb, TAG_init_response_TimeString)) {
        response->time_string.str = epb_get_string(&epb, TAG_init_response_TimeString, &response->time_string.len);
        response->has_time_string = true;
    }

    return response;
}

void epb_unpack_init_response_free(init_response *response)
{
    epb_unpack_base_response_free(response->base_response);
    vPortFree(response);
}

int epb_send_data_request_pack_size(send_data_request *request)
{
    int pack_size = 0;
    int len = 0;
    len = epb_base_request_pack_size(request->base_request);
    pack_size += epb_length_delimited_pack_size(TAG_send_data_request_base_request, len);
    pack_size += epb_length_delimited_pack_size(TAG_send_data_request_Data, request->data.len);
    if (request->has_type) {
        pack_size += epb_varint32_pack_size(TAG_send_data_request_Type, request->type, false);
    }
    return pack_size;
}

int epb_pack_send_data_request(send_data_request *request, uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_pack_init(&epb, buf, buf_len);

    int ret;
    int tmp_len;
    uint8_t *tmp;

    tmp_len = epb_base_request_pack_size(request->base_request);
    tmp = NULL;
    ret = epb_pack_base_request(request->base_request, tmp, tmp_len);
    if (ret < 0) {
        return ret;
    }
    ret = epb_set_message(&epb, TAG_send_data_request_base_request, tmp, tmp_len);
    if (ret < 0) return ret;
    ret = epb_set_bytes(&epb, TAG_send_data_request_Data, request->data.data, request->data.len);
    if (ret < 0) return ret;
    if (request->has_type) {
        ret = epb_set_enum(&epb, TAG_send_data_request_Type, request->type);
        if (ret < 0) return ret;
    }

    return epb_get_packed_size(&epb);
}

send_data_response *epb_unpack_send_data_response(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_send_data_response_BaseResponse)) {
        return NULL;
    }
    send_data_response *response = (send_data_response *)pvPortMalloc (sizeof(send_data_response));
    memset(response, 0, sizeof(send_data_response));
    tmp = epb_get_message(&epb, TAG_send_data_response_BaseResponse, &tmp_len);
    response->base_response = epb_unpack_base_response(tmp, tmp_len);
    if (response->base_response == NULL) {
        vPortFree(response);
        return NULL;
    }
    if (epb_has_tag(&epb, TAG_send_data_response_Data)) {
        response->data.data = epb_get_bytes(&epb, TAG_send_data_response_Data, &response->data.len);
        response->has_data = true;
    }
    return response;
}

void epb_unpack_send_data_response_free(send_data_response *response)
{
    epb_unpack_base_response_free(response->base_response);
    vPortFree(response);
}

recv_data_push *epb_unpack_recv_data_push(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_recv_data_push_BasePush)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_recv_data_push_Data)) {
        return NULL;
    }
    recv_data_push *push = (recv_data_push *)pvPortMalloc (sizeof(recv_data_push));
    memset(push, 0, sizeof(recv_data_push));
    tmp = epb_get_message(&epb, TAG_recv_data_push_BasePush, &tmp_len);
    push->base_push = epb_unpack_base_push(tmp, tmp_len);
    if (push->base_push == NULL) {
        vPortFree(push);
        return NULL;
    }
    push->data.data = epb_get_bytes(&epb, TAG_recv_data_push_Data, &push->data.len);
    if (epb_has_tag(&epb, TAG_recv_data_push_Type)) {
        push->type = (EmDeviceDataType)epb_get_enum(&epb, TAG_recv_data_push_Type);
        push->has_type = true;
    }
    return push;
}

void epb_unpack_recv_data_push_free(recv_data_push *push)
{
    epb_unpack_base_push_free(push->base_push);
    vPortFree(push);
}

switch_switch_view *epb_unpack_switch_switch_view(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_switch_switch_view_BasePush)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_switch_switch_view_SwitchViewOp)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_switch_switch_view_ViewId)) {
        return NULL;
    }
    switch_switch_view *push = (switch_switch_view *)pvPortMalloc (sizeof(switch_switch_view));
    memset(push, 0, sizeof(switch_switch_view));
    tmp = epb_get_message(&epb, TAG_switch_switch_view_BasePush, &tmp_len);
    push->base_push = epb_unpack_base_push(tmp, tmp_len);
    if (push->base_push == NULL) {
        vPortFree(push);
        return NULL;
    }
    push->switch_view_op = (EmSwitchViewOp)epb_get_enum(&epb, TAG_switch_switch_view_SwitchViewOp);
    push->view_id = (EmViewId)epb_get_enum(&epb, TAG_switch_switch_view_ViewId);
    return push;
}

void epb_unpack_switch_switch_view_free(switch_switch_view *push)
{
    epb_unpack_base_push_free(push->base_push);
    vPortFree(push);
}

switch_backgroud_push *epb_unpack_switch_backgroud_push(const uint8_t *buf, int buf_len)
{
    Epb epb;
    epb_unpack_init(&epb, buf, buf_len);
    const uint8_t *tmp;
    int tmp_len;
    if (!epb_has_tag(&epb, TAG_switch_backgroud_push_BasePush)) {
        return NULL;
    }
    if (!epb_has_tag(&epb, TAG_switch_backgroud_push_SwitchBackgroundOp)) {
        return NULL;
    }
    switch_backgroud_push *push = (switch_backgroud_push *)pvPortMalloc (sizeof(switch_backgroud_push));
    memset(push, 0, sizeof(switch_backgroud_push));
    tmp = epb_get_message(&epb, TAG_switch_backgroud_push_BasePush, &tmp_len);
    push->base_push = epb_unpack_base_push(tmp, tmp_len);
    if (push->base_push == NULL) {
        vPortFree(push);
        return NULL;
    }
    push->switch_background_op = (EmSwitchBackgroundOp)epb_get_enum(&epb, TAG_switch_backgroud_push_SwitchBackgroundOp);
    return push;
}

void epb_unpack_switch_backgroud_push_free(switch_backgroud_push *push)
{
    epb_unpack_base_push_free(push->base_push);
    vPortFree(push);
}


