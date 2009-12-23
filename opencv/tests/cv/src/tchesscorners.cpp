/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "cvtest.h"

class CV_ChessboardDetectorTimingTest : public CvTest
{
public:
    CV_ChessboardDetectorTimingTest();
protected:
    void run(int);
};


CV_ChessboardDetectorTimingTest::CV_ChessboardDetectorTimingTest():
    CvTest( "chessboard-detector-timing", "cvFindChessboardCorners" )
{
    support_testing_modes = CvTS::TIMING_MODE;
}

/* ///////////////////// chess_corner_test ///////////////////////// */
void CV_ChessboardDetectorTimingTest::run( int start_from )
{
    int code = CvTS::OK;

    /* test parameters */
    char   filepath[1000];
    char   filename[1000];

    CvMat*  _v = 0;
    CvPoint2D32f* v;

    IplImage* img = 0;
    IplImage* gray = 0;
    IplImage* thresh = 0;

    int  idx, max_idx;
    int  progress = 0;

    sprintf( filepath, "%scameracalibration/", ts->get_data_path() );
    sprintf( filename, "%schessboard_timing_list.dat", filepath );
    printf("Reading file %s\n", filename);
    CvFileStorage* fs = cvOpenFileStorage( filename, 0, CV_STORAGE_READ );
    CvFileNode* board_list = fs ? cvGetFileNodeByName( fs, 0, "boards" ) : 0;

    if( !fs || !board_list || !CV_NODE_IS_SEQ(board_list->tag) ||
        board_list->data.seq->total % 2 != 0 )
    {
        ts->printf( CvTS::LOG, "chessboard_timing_list.dat can not be readed or is not valid" );
        code = CvTS::FAIL_MISSING_TEST_DATA;
        goto _exit_;
    }

    max_idx = board_list->data.seq->total/2;

    for( idx = start_from; idx < max_idx; idx++ )
    {
        int count0 = -1;
        int count = 0;
        CvSize pattern_size = cvSize(5, 7);
        int result;
        
        ts->update_context( this, idx-1, true );

        /* read the image */
        const char* imgname = cvReadString((CvFileNode*)cvGetSeqElem(board_list->data.seq,idx*2),"dummy.txt");
        sprintf( filename, "%s%s", filepath, imgname );
    
        img = cvLoadImage( filename );
        
        if( !img )
        {
            ts->printf( CvTS::LOG, "one of chessboard images can't be read: %s\n", filename );
            if( max_idx == 1 )
            {
                code = CvTS::FAIL_MISSING_TEST_DATA;
                goto _exit_;
            }
            continue;
        }

        gray = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        thresh = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        cvCvtColor( img, gray, CV_BGR2GRAY );
 
        int is_chessboard = cvReadInt((CvFileNode*)cvGetSeqElem(board_list->data.seq,idx*2+1),0);

        count0 = pattern_size.width*pattern_size.height;

        /* allocate additional buffers */
        _v = cvCreateMat(1, count0, CV_32FC2);
        count = count0;

        v = (CvPoint2D32f*)_v->data.fl;

        int64 _time0 = cvGetTickCount();
        OPENCV_CALL( result = cvFindChessboardCorners(
                     gray, pattern_size, v, &count, 7 ));
        int64 _time1 = cvGetTickCount();

        //show_points( gray, 0, count0, v, count, pattern_size, result );
        if( result != is_chessboard )
        {
            ts->printf( CvTS::LOG, "Error: chessboard was %sdetected in the image %s\n", 
                       result ? "" : "not ", imgname );
            //code = CvTS::FAIL_INVALID_OUTPUT;
            //goto _exit_;
        }

        OPENCV_CALL( cvFindCornerSubPix( gray, v, count, cvSize( 5, 5 ), cvSize(-1,-1),
                            cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER,30,0.1)));
        
        ts->printf(CvTS::LOG, "%s: chessboard %d, findChessboard time us per pixel: %f\n", 
                   imgname, is_chessboard, float(_time1 - _time0)/cvGetTickFrequency()/(gray->width*gray->height));
        //show_points( gray, u + 1, count0, v, count, pattern_size, result  );

        cvReleaseMat( &_v );
        cvReleaseImage( &img );
        cvReleaseImage( &gray );
        cvReleaseImage( &thresh );
        progress = update_progress( progress, idx-1, max_idx, 0 );
    }

_exit_:

    /* release occupied memory */
    cvReleaseMat( &_v );
    cvReleaseFileStorage( &fs );
    cvReleaseImage( &img );
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );

    if( code < 0 )
        ts->set_failed_test_info( code );
}

CV_ChessboardDetectorTimingTest chessboard_detector_timing_test;

/* End of file. */
