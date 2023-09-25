#ifndef MBEDTLS_BIO_FREERTOS_H
#define MBEDTLS_BIO_FREERTOS_H

int mbedtls_platform_send( void * ctx,
                           const unsigned char * buf,
                           size_t len );

int mbedtls_platform_recv( void * ctx,
                           unsigned char * buf,
                           size_t len );

#endif