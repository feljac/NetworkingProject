#include <malloc.h>
#include <string.h>
#include <netinet/in.h>
#include "zlib-1.2.11/zlib.h"
#include "packet_interface.h"

pkt_t* pkt_new()
{
    pkt_t *pkt = calloc(1, sizeof(pkt_t));
    return pkt;
}

void pkt_del(pkt_t *pkt)
{
    if(pkt->payload){
        free(pkt->payload);
    }
    free(pkt);
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
    int read = 0;

    if(len < sizeof(pkt->header)){
        fprintf(stderr, "length header invalid\n");
        return E_NOMEM;
    }

    memcpy(pkt, data, sizeof(uint16_t));
    read += sizeof(uint16_t);

    memcpy(&(pkt->header.length), data + read, sizeof(pkt->header.length));
    pkt->header.length = ntohs(pkt->header.length);
    read += sizeof(pkt->header.length);

    memcpy(&(pkt->header.timestamp), data + read, sizeof(pkt->header.timestamp));
    read += sizeof(pkt->header.timestamp);

    memcpy(&(pkt->header.crc1), data + read, sizeof(pkt->header.crc1));
    read += sizeof(pkt->header.crc1);

    if(!pkt_get_tr(pkt) && pkt_get_length(pkt) != 0){
        if(len < (sizeof(pkt->header) + pkt_get_length(pkt) + sizeof(pkt->crc2))){
            fprintf(stderr, "length payload invalid\n");
            return E_NOMEM;
        }
        pkt_set_payload(pkt, data + read, pkt_get_length(pkt));
        read += pkt_get_length(pkt);
        memcpy(&(pkt->crc2), data + read, sizeof(pkt->crc2));
        read += sizeof(pkt->crc2);
    }

    uLong crc1, crc2;
    crc1 = crc32(0L, Z_NULL, 0);
    crc2 = crc32(0L, Z_NULL, 0);
    size_t length = read;
    if(pkt_get_tr(pkt) == 1){
        pkt_set_tr(pkt, 0);
        char* temp_buf = malloc(read);
        pkt_encode(pkt, temp_buf, &length);
        crc1 = crc32(crc1, (Bytef*) temp_buf, sizeof(pkt->header) - sizeof(uint32_t));
        pkt_set_tr(pkt, 1);
    }
    else{
        crc1 = crc32(crc1, (Bytef*) data, sizeof(pkt->header) - sizeof(uint32_t));
    }
    if(htonl((uint32_t)crc1) != pkt_get_crc1(pkt)){
        fprintf(stderr, "crc1 invalid\n");
        return E_CRC;
    }

    crc2 = crc32(crc2, (Bytef*) pkt->payload, pkt_get_length(pkt));
    if(htonl((uint32_t)crc2) != pkt_get_crc2(pkt)){
        fprintf(stderr, "crc2 invalid\n");
        return E_CRC;
    }

    return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    int write = 0;

    if(*len < sizeof(pkt->header)){
        fprintf(stderr,"encode :length header invalid\n");
        return E_NOMEM;
    }

    memcpy(buf, pkt, sizeof(uint16_t));
    write += sizeof(uint16_t);

    uint16_t length = htons(pkt->header.length);

    memcpy(buf + write, &(length), sizeof(pkt->header.length));
    write += sizeof(pkt->header.length);

    memcpy(buf + write, &(pkt->header.timestamp), sizeof(pkt->header.timestamp));
    write += sizeof(pkt->header.timestamp);

    uLong crc1, crc2;
    uint32_t crc;
    crc1 = crc32(0L, Z_NULL, 0);
    crc2 = crc32(0L, Z_NULL, 0);

    crc1 = crc32(crc1, (Bytef*) buf, write);
    crc = htonl((uint32_t)crc1);
    memcpy(buf + write, &crc, sizeof(crc));
    write += sizeof(crc);

    if(pkt_get_length(pkt) != 0){
        if(*len < (sizeof(pkt->header) + pkt_get_length(pkt) + sizeof(pkt->crc2))){
            fprintf(stderr,"encode :length payload invalid\n");
            return E_NOMEM;
        }
        memcpy(buf + write, pkt->payload, pkt_get_length(pkt));
        write += pkt_get_length(pkt);
        crc2 = crc32(crc2, (Bytef*) pkt->payload, pkt_get_length(pkt));
        crc = htonl((uint32_t)crc2);
        memcpy(buf + write, &crc, sizeof(crc));
        write += sizeof(crc);
    }
    *len = (size_t)write;
    return PKT_OK;
}

ptypes_t pkt_get_type  (const pkt_t* pkt)
{
    return pkt->header.type;
}

uint8_t  pkt_get_tr(const pkt_t* pkt)
{
    return pkt->header.tr;
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
    return pkt->header.window;
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
    return pkt->header.seqnum;
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
    return pkt->header.length;
}

uint32_t pkt_get_timestamp   (const pkt_t* pkt)
{
    return pkt->header.timestamp;
}

uint32_t pkt_get_crc1   (const pkt_t* pkt)
{
    return pkt->header.crc1;
}

uint32_t pkt_get_crc2   (const pkt_t* pkt)
{
    return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    return pkt->payload;
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
    if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK)
        return E_TYPE;
    pkt->header.type = type;
    return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
    if(tr > 1)
        return E_TR;
    pkt->header.tr = tr;
    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    pkt->header.window = window;
    return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    pkt->header.seqnum = seqnum;
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
    if(length > MAX_PAYLOAD_SIZE)
        return E_LENGTH;
    pkt->header.length = length;
    return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
    pkt->header.timestamp = timestamp;
    return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
    pkt->header.crc1 = crc1;
    return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
    pkt->crc2 = crc2;
    return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length)
{
    if(!data){
        pkt->payload = NULL;
        return pkt_set_length(pkt, 0);
    }

    if((pkt->payload = calloc(1, length)) == NULL){
        return E_NOMEM;
    }
    memcpy(pkt->payload, data, length);

    return pkt_set_length(pkt, length);
}