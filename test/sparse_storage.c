/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://www.hdfgroup.org/licenses.               *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:    Tests sparse storage based on the concept of structured chunk
 */
#define H5D_FRIEND /*suppress error about including H5Dpkg      */
#define H5D_TESTING
#define H5FD_FRIEND /*suppress error about including H5FDpkg      */
#define H5FD_TESTING

#define H5Z_FRIEND /*suppress error about including H5Zpkg      */

#include "testhdf5.h"
#include "H5srcdir.h"

#include "H5CXprivate.h" /* API Contexts                         */
#include "H5Iprivate.h"
#include "H5Pprivate.h"

#define H5F_FRIEND /*suppress error about including H5Fpkg */
#define H5F_TESTING
#include "H5Fpkg.h" /* File access                          */

#define H5S_FRIEND  /*suppress error about including H5Spkg */
#include "H5Spkg.h" /* Dataspace                            */

#define H5T_FRIEND  /*suppress error about including H5Tpkg */
#include "H5Tpkg.h" /* Datatype                             */

#define H5A_FRIEND  /*suppress error about including H5Apkg     */
#include "H5Apkg.h" /* Attributes                   */

/* Use in version bound test */
#define H5O_FRIEND  /*suppress error about including H5Opkg */
#include "H5Opkg.h" /* Object headers                       */

#include "H5Dpkg.h"
#include "H5FDpkg.h"
#include "H5VMprivate.h"
#include "H5Zpkg.h"

static const char *FILENAME[] = {"sparse", /* 0 */
                                 NULL};
#define FILENAME_BUF_SIZE 1024

#define SPARSE_DSET "sparse_dset"

#define RANK     2
#define NX       10
#define NY       10
#define CHUNK_NX 5
#define CHUNK_NY 5

/* Size of a chunk */
#define CHK_SIZE (CHUNK_NX * CHUNK_NY * sizeof(int))


/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests for sparse data
 *              Tests are copied and modified from:
 *               --test/dsets.c
 *               --test/direct_chunk.c
 *               --test/chunk_info.c
 *
 * Return:      EXIT_SUCCESS/EXIT_FAILURE
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    char        filename[FILENAME_BUF_SIZE];
    hid_t       file, grp, fapl, fapl2;
    hid_t       fcpl = H5I_INVALID_HID, fcpl2 = H5I_INVALID_HID;
    unsigned    new_format;
    unsigned    paged;
    unsigned    minimized_ohdr;
    int         mdc_nelmts;
    size_t      rdcc_nelmts;
    size_t      rdcc_nbytes;
    double      rdcc_w0;
    int         nerrors = 0;
    const char *driver_name;
    bool        contig_addr_vfd; /* Whether VFD used has a contiguous address space */
    bool        driver_is_default_compatible;

    /* Don't run this test using certain file drivers */
    driver_name = h5_get_test_driver_name();

    /* Current VFD that does not support contiguous address space */
    contig_addr_vfd = (bool)(strcmp(driver_name, "split") != 0 && strcmp(driver_name, "multi") != 0);

    /* Set the random # seed */
    srand((unsigned)time(NULL));

    /* Testing setup */
    h5_test_init();
    fapl = h5_fileaccess();

    if (h5_driver_is_default_vfd_compatible(fapl, &driver_is_default_compatible) < 0)
        TEST_ERROR;

    /* Turn off the chunk cache, so all the chunks are immediately written to disk */
    if (H5Pget_cache(fapl, &mdc_nelmts, &rdcc_nelmts, &rdcc_nbytes, &rdcc_w0) < 0)
        goto error;
    rdcc_nbytes = 0;
    if (H5Pset_cache(fapl, mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0) < 0)
        goto error;

    /* Copy the file access property list */
    if ((fapl2 = H5Pcopy(fapl)) < 0)
        TEST_ERROR;

    /* Set the "use the latest version of the format" bounds for creating objects in the file */
    if (H5Pset_libver_bounds(fapl2, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0)
        TEST_ERROR;

    /* create a file creation property list */
    if ((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;
    if ((fcpl2 = H5Pcopy(fcpl)) < 0)
        TEST_ERROR;

    /* Set file space strategy to paged aggregation and persisting free-space */
    if (H5Pset_file_space_strategy(fcpl2, H5F_FSPACE_STRATEGY_PAGE, true, (hsize_t)1) < 0)
        TEST_ERROR;

    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);

    /* Test with paged aggregation enabled or not */
    for (paged = false; paged <= true; paged++) {

        /* Temporary: skip testing for multi/split drivers:
             fail file create when persisting free-space or using paged aggregation strategy */
        if (!contig_addr_vfd && paged)
            continue;

        for (minimized_ohdr = false; minimized_ohdr <= true; minimized_ohdr++) {

            /* Test with old & new format groups */
            for (new_format = false; new_format <= true; new_format++) {
                hid_t my_fapl, my_fcpl;

                /* Set the FAPL for the type of format */
                if (new_format) {
                    my_fapl = fapl2;
                    if (paged) {
                        my_fcpl = fcpl2;
                        puts("\nTesting with new file format and paged aggregation");
                    }
                    else {
                        my_fcpl = fcpl;
                        puts("\nTesting with new file format and non-paged aggregation");
                    }
                } /* end if */
                else {
                    my_fapl = fapl;
                    if (paged) {
                        my_fcpl = fcpl2;
                        puts("Testing with old file format and paged aggregation:");
                    }
                    else {
                        my_fcpl = fcpl;
                        puts("Testing with old file format and non-paged aggregation:");
                    }
                } /* end else */

                /* Create the file for this test */
                if ((file = H5Fcreate(filename, H5F_ACC_TRUNC, my_fcpl, my_fapl)) < 0)
                    goto error;

                if (true == minimized_ohdr) {
                    if (0 > H5Fset_dset_no_attrs_hint(file, true))
                        goto error;
                    puts("(minimized dataset object headers with file setting)");
                }

                /* Cause the library to emit initial messages */
                if ((grp = H5Gcreate2(file, "emit diagnostics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
                    goto error;
                if (H5Oset_comment(grp, "Causes diagnostic messages to be emitted") < 0)
                    goto error;
                if (H5Gclose(grp) < 0)
                    goto error;

                if (H5Fclose(file) < 0)
                    goto error;
            } /* end for new_format */
        }     /* end for minimized_ohdr */
    }         /* end for paged */

    /* Close property lists */
    if (H5Pclose(fapl2) < 0)
        TEST_ERROR;
    if (H5Pclose(fcpl) < 0)
        TEST_ERROR;
    if (H5Pclose(fcpl2) < 0)
        TEST_ERROR;

    /* Verify symbol table messages are cached */
    nerrors += (h5_verify_cached_stabs(FILENAME, fapl) < 0 ? 1 : 0);

    if (nerrors)
        goto error;
    printf("All sparse storage tests passed.\n");

    h5_cleanup(FILENAME, fapl);

    exit(EXIT_SUCCESS);

error:
    nerrors = MAX(1, nerrors);
    printf("***** %d SPARSE STORAGE TEST%s FAILED! *****\n", nerrors, 1 == nerrors ? "" : "S");
    exit(EXIT_FAILURE);
} /* end main() */
