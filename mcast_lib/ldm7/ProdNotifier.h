/**
 * Copyright 2014 University Corporation for Atmospheric Research. All rights
 * reserved. See the the file COPYRIGHT in the top-level source-directory for
 * licensing conditions.
 *
 * @file PerProdNotifier.h
 *
 * This file declares the API for a class that notifies the receiving
 * application of events on a per-product basis.
 *
 * @author: Steven R. Emmerson
 */

#ifndef PROD_NOTIFIER_H_
#define PROD_NOTIFIER_H_

#include "fmtp.h"
#include "pq.h"

#include <sys/types.h>
#include "MldmRcvr.h"

/**
 * @retval         0            Success. `*data` is set. If `NULL`, then
 *                              data-product is already in LDM product-queue.
 * @retval         -1           Failure. `log_add()` called.
 */
typedef int     (*BopFunc)(
                    Mlr* const restrict        mlr,
                    const size_t               prodSize,
                    const void* const restrict metadata,
                    const unsigned             metaSize,
                    void** const restrict      data,
                    pqe_index* const restrict  pqeIndex);
typedef int     (*EopFunc)(
                    Mlr* const restrict             mlr,
                    void* const restrict            prod,
                    const size_t                    prodSize,
                    const pqe_index* const restrict pqeIndex,
                    const double                    duration);
typedef void    (*MissedProdFunc)(
                    void* const restrict            obj,
                    const FmtpProdIndex             iProd,
                    const pqe_index* const restrict pqeIndex);

#ifdef __cplusplus

#include "RecvProxy.h"

#include <mutex>
#include <unordered_map>

class ProdNotifier: public RecvProxy {
public:
    /**
     * Constructs from the notification functions.
     *
     * @param[in] bop_func              Function to call when the beginning of
     *                                  a product has been seen by the FMTP
     *                                  layer.
     * @param[in] eop_func              Function to call when a product has been
     *                                  completely received by the FMTP layer.
     * @param[in] missed_prod_func      Function to call when a product is
     *                                  missed by the FMTP layer.
     * @param[in] mlr                   Associated multicast LDM receiver.
     * @throws    std::invalid_argument if @code{!bof_func || !eof_func ||
     *                                  !missed_prod_func}
     */
    ProdNotifier(
            BopFunc         bop_func,
            EopFunc         eop_func,
            MissedProdFunc  missed_prod_func,
            Mlr*            mlr);

    ~ProdNotifier() {}
    void startProd(
            const struct timespec& startTime,
            const FmtpProdIndex    iProd,
            size_t                 prodSize,
            void*                  metadata,
            unsigned               metaSize,
            void**                 data);
    /**
     * @param[in] stopTime         When end-of-product message arrived
     * @param[in] prodIndex        The FMTP index of the product.
     * @throws std::runtime_error  Receiving application error.
     */
    void endProd(
            const struct timespec& stopTime,
            const FmtpProdIndex    prodIndex);
    void missedProd(FmtpProdIndex prodIndex);

private:
    /**
     * Mutex to ensure thread-safety because an instance is called by both the
     * unicast- and multicast-receiving threads.
     */
    std::mutex          mutex;
    /**
     * C function to call when a beginning-of-product has been seen by the FMTP
     * layer.
     */
    BopFunc             bop_func;
    /**
     * C function to call when a product has been completely received by the
     * FMTP layer.
     */
    EopFunc             eop_func;
    /**
     * C function to call when a product is missed by the FMTP layer.
     */
    MissedProdFunc      missed_prod_func;
    /**
     * Associated multicast LDM receiver.
     */
    Mlr*               mlr;

    // Map from product-index to useful information.
    class ProdInfo {
    public:
        ProdInfo();
        ~ProdInfo();

        /// Pointer to start of XDR-encoded product in product-queue
        void*           pqRegion;
        /// Time of start-of-transmission
        struct timespec startTime;
        /// Size of XDR-encoded product in bytes
        size_t          size;
        /// Reference to allocated space in product-queue
        pqe_index       index;
    };
    std::unordered_map<FmtpProdIndex, ProdInfo> prodInfos;
};

#endif

#ifdef __cplusplus
    extern "C" {
    #define restrict
#endif

int ppn_new(
        void**            ppn,
        BopFunc           bop_func,
        EopFunc           eop_func,
        MissedProdFunc    missed_prod_func,
        Mlr*              obj);

void ppn_free(
        void* ppn);

#ifdef __cplusplus
    }
#endif

#endif /* PROD_NOTIFIER_H_ */
