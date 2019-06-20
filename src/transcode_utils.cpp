/******************************************************************************\
Copyright (c) 2005-2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#include "mfx_samples_config.h"
#include "plugin_utils.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#include <d3d9.h>
#include "d3d_allocator.h"
#include "d3d11_allocator.h"
#else
#include <stdarg.h>
#include "vaapi_allocator.h"
#endif

#include "sysmem_allocator.h"
#include "transcode_utils.h"

using namespace TranscodingSample;

// parsing defines
#define IS_SEPARATOR(ch)  ((ch) <= ' ' || (ch) == '=')
#define VAL_CHECK(val, argIdx, argName) \
{ \
    if (val) \
    { \
        PrintError(MSDK_STRING("Input argument number %d \"%s\" require more parameters"), argIdx, argName); \
        return MFX_ERR_UNSUPPORTED;\
    } \
}

#define SIZE_CHECK(cond) \
{ \
    if (cond) \
    { \
        PrintError(MSDK_STRING("Buffer is too small")); \
        return MFX_ERR_UNSUPPORTED; \
    } \
}

msdk_tick TranscodingSample::GetTick()
{
    return msdk_time_get_tick();
}

mfxF64 TranscodingSample::GetTime(msdk_tick start)
{
    static msdk_tick frequency = msdk_time_get_frequency();

    return MSDK_GET_TIME(msdk_time_get_tick(), start, frequency);
}

void TranscodingSample::PrintError(const msdk_char *strErrorMessage, ...)
{
    if (strErrorMessage)
    {
        msdk_printf(MSDK_STRING("ERROR: "));
        va_list args;
        va_start(args, strErrorMessage);
        msdk_vprintf(strErrorMessage, args);
        va_end(args);
        msdk_printf(MSDK_STRING("\nUsage: sample_multi_transcode [options] [--] pipeline-description\n"));
        msdk_printf(MSDK_STRING("   or: sample_multi_transcode [options] -par ParFile\n"));
        msdk_printf(MSDK_STRING("\n"));
        msdk_printf(MSDK_STRING("Run application with -? option to get full help text.\n\n"));
    }
}

void TranscodingSample::PrintHelp()
{
    msdk_printf(MSDK_STRING("Multi Transcoding Sample Version %s\n\n"), MSDK_SAMPLE_VERSION);
    msdk_printf(MSDK_STRING("Command line parameters\n"));

    msdk_printf(MSDK_STRING("Usage: sample_multi_transcode [options] [--] pipeline-description\n"));
    msdk_printf(MSDK_STRING("   or: sample_multi_transcode [options] -par ParFile\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("  -stat <N>\n"));
    msdk_printf(MSDK_STRING("                Output statistic every N transcoding cycles\n"));
    msdk_printf(MSDK_STRING("Options:\n"));
    //                     ("  ............xx
    msdk_printf(MSDK_STRING("  -?            Print this help and exit\n"));
    msdk_printf(MSDK_STRING("  -p <file-name>\n"));
    msdk_printf(MSDK_STRING("                Collect performance statistics in specified file\n"));
    msdk_printf(MSDK_STRING("  -timeout <seconds>\n"));
    msdk_printf(MSDK_STRING("                Set time to run transcoding in seconds\n"));
    msdk_printf(MSDK_STRING("  -greedy \n"));
    msdk_printf(MSDK_STRING("                Use greedy formula to calculate number of surfaces\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Pipeline description (general options):\n"));
    msdk_printf(MSDK_STRING("  -i::h265|h264|mpeg2|vc1|mvc|jpeg|vp8 <file-name>\n"));
    msdk_printf(MSDK_STRING("                Set input file and decoder type\n"));
    msdk_printf(MSDK_STRING("  -o::h265|h264|mpeg2|mvc|jpeg|vp8|raw <file-name>\n"));
    msdk_printf(MSDK_STRING("                Set output file and encoder type\n"));
    msdk_printf(MSDK_STRING("  -sw|-hw|-hw_d3d11\n"));
    msdk_printf(MSDK_STRING("                SDK implementation to use: \n"));
    msdk_printf(MSDK_STRING("                      -hw - platform-specific on default display adapter (default)\n"));
    msdk_printf(MSDK_STRING("                      -hw_d3d11 - platform-specific via d3d11\n"));
    msdk_printf(MSDK_STRING("                      -sw - software\n"));
    msdk_printf(MSDK_STRING("  -async        Depth of asynchronous pipeline. default value 1\n"));
    msdk_printf(MSDK_STRING("  -join         Join session with other session(s), by default sessions are not joined\n"));
    msdk_printf(MSDK_STRING("  -priority     Use priority for join sessions. 0 - Low, 1 - Normal, 2 - High. Normal by default\n"));
    msdk_printf(MSDK_STRING("  -threads num  Number of session internal threads to create\n"));
    msdk_printf(MSDK_STRING("  -n            Number of frames to transcode \n"));
    msdk_printf(MSDK_STRING("  -ext_allocator    Force usage of external allocators\n"));
    msdk_printf(MSDK_STRING("  -sys          Force usage of external system allocator\n"));
    msdk_printf(MSDK_STRING("  -fps <frames per second>\n"));
    msdk_printf(MSDK_STRING("                Transcoding frame rate limit\n"));
    msdk_printf(MSDK_STRING("  -pe           Set encoding plugin for this particular session.\n"));
    msdk_printf(MSDK_STRING("                This setting overrides plugin settings defined by SET clause.\n"));
    msdk_printf(MSDK_STRING("  -pd           Set decoding plugin for this particular session.\n"));
    msdk_printf(MSDK_STRING("                This setting overrides plugin settings defined by SET clause.\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Pipeline description (encoding options):\n"));
    MOD_SMT_PRINT_HELP;
    msdk_printf(MSDK_STRING("  -b <Kbits per second>\n"));
    msdk_printf(MSDK_STRING("                Encoded bit rate, valid for H.264, MPEG2 and MVC encoders\n"));
    msdk_printf(MSDK_STRING("  -f <frames per second>\n"));
    msdk_printf(MSDK_STRING("                Video frame rate for the whole pipeline, overwrites input stream's framerate is taken\n"));
    msdk_printf(MSDK_STRING("  -fe <frames per second>\n"));
    msdk_printf(MSDK_STRING("                Video frame rate for the output of the pipeline.\n"));
    msdk_printf(MSDK_STRING("                It affects VPP output (if VPP is enabled) or/and encoder framerate.\n"));
    msdk_printf(MSDK_STRING("                If this option is omitted, -f sets both input and output framerate.\n"));
    msdk_printf(MSDK_STRING("  -u 1|4|7      Target usage: quality (1), balanced (4) or speed (7); valid for H.264, MPEG2 and MVC encoders. Default is balanced\n"));
    msdk_printf(MSDK_STRING("  -q <quality>  Quality parameter for JPEG encoder; in range [1,100], 100 is the best quality\n"));
    msdk_printf(MSDK_STRING("  -l numSlices  Number of slices for encoder; default value 0 \n"));
    msdk_printf(MSDK_STRING("  -mss maxSliceSize \n"));
    msdk_printf(MSDK_STRING("                Maximum slice size in bytes. Supported only with -hw and h264 codec. This option is not compatible with -l option.\n"));
    msdk_printf(MSDK_STRING("  -la           Use the look ahead bitrate control algorithm (LA BRC) for H.264 encoder. Supported only with -hw option on 4th Generation Intel Core processors. \n"));
    msdk_printf(MSDK_STRING("  -lad depth    Depth parameter for the LA BRC, the number of frames to be analyzed before encoding. In range [10,100]. \n"));
    msdk_printf(MSDK_STRING("                May be 1 in the case when -mss option is specified \n"));
    msdk_printf(MSDK_STRING("  -hrd <KB>     Maximum possible size of any compressed frames \n"));
    msdk_printf(MSDK_STRING("  -wb <KBps>    Maximum bitrate for sliding window \n"));
    msdk_printf(MSDK_STRING("  -ws           Sliding window size in frames\n"));
    msdk_printf(MSDK_STRING("  -gop_size     Size of GOP structure in frames \n"));
    msdk_printf(MSDK_STRING("  -dist         Distance between I- or P- key frames \n"));
    msdk_printf(MSDK_STRING("  -num_ref      Number of reference frames\n"));
    msdk_printf(MSDK_STRING("  -gpucopy::<on,off> Enable or disable GPU copy mode\n"));
    msdk_printf(MSDK_STRING("  -cqp          Constant quantization parameter (CQP BRC) bitrate control method\n"));
    msdk_printf(MSDK_STRING("                              (by default constant bitrate control method is used), should be used along with -qpi, -qpp, -qpb.\n"));
    msdk_printf(MSDK_STRING("  -qpi          Constant quantizer for I frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("  -qpp          Constant quantizer for P frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("  -qpb          Constant quantizer for B frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("  -qsv-ff       Enable QSV-FF mode\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Pipeline description (vpp options):\n"));
    msdk_printf(MSDK_STRING("  -deinterlace             Forces VPP to deinterlace input stream\n"));
    msdk_printf(MSDK_STRING("  -deinterlace::ADI        Forces VPP to deinterlace input stream using ADI algorithm\n"));
    msdk_printf(MSDK_STRING("  -deinterlace::ADI_SCD    Forces VPP to deinterlace input stream using ADI_SCD algorithm\n"));
    msdk_printf(MSDK_STRING("  -deinterlace::ADI_NO_REF Forces VPP to deinterlace input stream using ADI no ref algorithm\n"));
    msdk_printf(MSDK_STRING("  -deinterlace::BOB        Forces VPP to deinterlace input stream using BOB algorithm\n"));
    msdk_printf(MSDK_STRING("  -detail <level>          Enables detail (edge enhancement) filter with provided level(0..100)\n"));
    msdk_printf(MSDK_STRING("  -denoise <level>         Enables denoise filter with provided level (0..100)\n"));
    msdk_printf(MSDK_STRING("  -FRC::PT      Enables FRC filter with Preserve Timestamp algorithm\n"));
    msdk_printf(MSDK_STRING("  -FRC::DT      Enables FRC filter with Distributed Timestamp algorithm\n"));
    msdk_printf(MSDK_STRING("  -FRC::INTERP  Enables FRC filter with Frame Interpolation algorithm\n"));
    msdk_printf(MSDK_STRING("     NOTE: -FRC filters work with -i::source pipelines only !!!\n"));
    msdk_printf(MSDK_STRING("  -ec::nv12|rgb4|yuy2|nv16|p010|p210   Forces encoder input to use provided chroma mode\n"));
    msdk_printf(MSDK_STRING("  -dc::nv12|rgb4|yuy2   Forces decoder output to use provided chroma mode\n"));
    msdk_printf(MSDK_STRING("     NOTE: chroma transform VPP may be automatically enabled if -ec/-dc parameters are provided\n"));
    msdk_printf(MSDK_STRING("  -angle 180    Enables 180 degrees picture rotation user module before encoding\n"));
    msdk_printf(MSDK_STRING("  -opencl       Uses implementation of rotation plugin (enabled with -angle option) through Intel(R) OpenCL\n"));
    msdk_printf(MSDK_STRING("  -w            Destination picture width, invokes VPP resize\n"));
    msdk_printf(MSDK_STRING("  -h            Destination picture height, invokes VPP resize\n"));
    msdk_printf(MSDK_STRING("  -field_processing t2t|t2b|b2t|b2b|fr2fr - Field Copy feature\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp <sourcesNum>      Enables composition from several decoding sessions. Result is written to the file\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp_only <sourcesNum> Enables composition from several decoding sessions. Result is shown on screen\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp_dst_x             X position of this stream in composed stream (should be used in decoder session)\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp_dst_y             Y position of this stream in composed stream (should be used in decoder session)\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp_dst_h             Height of this stream in composed stream (should be used in decoder session)\n"));
    msdk_printf(MSDK_STRING("  -vpp_comp_dst_w             Width of this stream in composed stream (should be used in decoder session)\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("ParFile format:\n"));
    msdk_printf(MSDK_STRING("  ParFile is extension of what can be achieved by setting pipeline in the command\n"));
    msdk_printf(MSDK_STRING("line. For more information on ParFile format see readme-multi-transcode.pdf\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Examples:\n"));
    msdk_printf(MSDK_STRING("  sample_multi_transcode -i::mpeg2 in.mpeg2 -o::h264 out.h264\n"));
    msdk_printf(MSDK_STRING("  sample_multi_transcode -i::mvc in.mvc -o::mvc out.mvc -w 320 -h 240\n"));
}

void TranscodingSample::PrintInfo(mfxU32 session_number, sInputParams* pParams, mfxVersion *pVer)
{
    msdk_char buf[2048];
    MSDK_CHECK_POINTER_NO_RET(pVer);

    if ((MFX_IMPL_AUTO <= pParams->libType) && (MFX_IMPL_HARDWARE4 >= pParams->libType))
    {
        msdk_printf(MSDK_STRING("MFX %s Session %d API ver %d.%d parameters: \n"),
            (MFX_IMPL_SOFTWARE == pParams->libType)? MSDK_STRING("SOFTWARE") : MSDK_STRING("HARDWARE"),
                 session_number,
                 pVer->Major,
                 pVer->Minor);
    }

    if (0 == pParams->DecodeId)
        msdk_printf(MSDK_STRING("Input  video: From parent session\n"));
    else
        msdk_printf(MSDK_STRING("Input  video: %s\n"), CodecIdToStr(pParams->DecodeId).c_str());

    // means that source is parent session
    if (0 == pParams->EncodeId)
        msdk_printf(MSDK_STRING("Output video: To child session\n"));
    else
        msdk_printf(MSDK_STRING("Output video: %s\n"), CodecIdToStr(pParams->EncodeId).c_str());
    if (PrintDllInfo(buf, MSDK_ARRAY_LEN(buf), pParams))
        msdk_printf(MSDK_STRING("MFX dll: %s\n"),buf);
    msdk_printf(MSDK_STRING("\n"));
}

bool TranscodingSample::PrintDllInfo(msdk_char* buf, mfxU32 buf_size, sInputParams* pParams)
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE   hCurrent = GetCurrentProcess();
    HMODULE *pModules;
    DWORD    cbNeeded;
    int      nModules;
    if (NULL == EnumProcessModules(hCurrent, NULL, 0, &cbNeeded))
        return false;

    nModules = cbNeeded / sizeof(HMODULE);

    pModules = new HMODULE[nModules];
    if (NULL == pModules)
    {
        return false;
    }
    if (NULL == EnumProcessModules(hCurrent, pModules, cbNeeded, &cbNeeded))
    {
        delete []pModules;
        return false;
    }

    for (int i = 0; i < nModules; i++)
    {
        GetModuleFileName(pModules[i], buf, buf_size);
        if (_tcsstr(buf, MSDK_STRING("libmfxhw")) && (MFX_IMPL_SOFTWARE != pParams->libType))
        {
            delete []pModules;
            return true;
        }
        else if (_tcsstr(buf, MSDK_STRING("libmfxsw")) && (MFX_IMPL_SOFTWARE == pParams->libType))
        {
            delete []pModules;
            return true;
        }

    }
    delete []pModules;
    return false;
#else
    return false;
#endif
}

CmdProcessor::CmdProcessor()
{
    m_SessionParamId = 0;
    m_SessionArray.clear();
    m_decoderPlugins.clear();
    m_encoderPlugins.clear();
    m_PerfFILE = NULL;
    m_parName = NULL;
    m_nTimeout = 0;
    statisticsWindowSize = 0;
    shouldUseGreedyFormula=false;

} //CmdProcessor::CmdProcessor()

CmdProcessor::~CmdProcessor()
{
    m_SessionArray.clear();
    m_decoderPlugins.clear();
    m_encoderPlugins.clear();
    if (m_PerfFILE)
        fclose(m_PerfFILE);

} //CmdProcessor::~CmdProcessor()

void CmdProcessor::PrintParFileName()
{
    if (m_parName && m_PerfFILE)
    {
        msdk_fprintf(m_PerfFILE, MSDK_STRING("Input par file: %s\n\n"), m_parName);
    }
}

mfxStatus CmdProcessor::ParseCmdLine(int n)
{
	mfxStatus sts = MFX_ERR_NONE;
	for(int i=0;i<3;i++)
		sts = ParseParamsForOneSession(i,n);
		

	return sts;

} //mfxStatus CmdProcessor::ParseCmdLine(int argc, msdk_char *argv[])

mfxStatus CmdProcessor::ParseParFile(FILE *parFile)
{
    mfxStatus sts = MFX_ERR_UNSUPPORTED;
    if (!parFile)
        return MFX_ERR_UNSUPPORTED;

    mfxU32 currPos = 0;
    mfxU32 lineIndex = 0;

    // calculate file size
    fseek(parFile, 0, SEEK_END);
    mfxU32 fileSize = ftell(parFile) + 1;
    fseek(parFile, 0, SEEK_SET);

    // allocate buffer for parsing
    s_ptr<msdk_char, false> parBuf;
    parBuf.reset(new msdk_char[fileSize]);
    msdk_char *pCur;

    while(currPos < fileSize)
    {
        pCur = /*_fgetts*/msdk_fgets(parBuf.get(), fileSize, parFile);
        if (!pCur)
            return MFX_ERR_NONE;
        while(pCur[currPos] != '\n' && pCur[currPos] != 0)
        {
            currPos++;
            if  (pCur + currPos >= parBuf.get() + fileSize)
                return sts;
        }
        // zero string
        if (!currPos)
            continue;

        sts = TokenizeLine(pCur, currPos);
//        if (MFX_ERR_NONE != sts)
//            PrintError(MSDK_STRING("Error in par file parameters at line %d"), lineIndex);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        currPos = 0;
        lineIndex++;
    }

    return MFX_ERR_NONE;

} //mfxStatus CmdProcessor::ParseParFile(FILE *parFile)

mfxStatus CmdProcessor::TokenizeLine(msdk_char *pLine, mfxU32 length)
{
    mfxU32 i;
    const mfxU8 maxArgNum = 255;
    msdk_char *argv[maxArgNum+1];
    mfxU32 argc = 0;
    s_ptr<msdk_char, false> pMemLine;

    pMemLine.reset(new msdk_char[length+2]);

    msdk_char *pTempLine = pMemLine.get();
    pTempLine[0] = ' ';
    pTempLine++;

    MSDK_MEMCPY_BUF(pTempLine,0 , length*sizeof(msdk_char), pLine, length*sizeof(msdk_char));

    // parse into command streams
    for (i = 0; i < length ; i++)
    {
        // check if separator
        if (IS_SEPARATOR(pTempLine[-1]) && !IS_SEPARATOR(pTempLine[0]))
        {
            argv[argc++] = pTempLine;
            if (argc > maxArgNum)
            {
                PrintError(MSDK_STRING("Too many parameters (reached maximum of %d)"), maxArgNum);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        if (*pTempLine == ' ' || *pTempLine == '\r' || *pTempLine == '\n')
        {
            *pTempLine = 0;
        }
        pTempLine++;
    }

    // EOL for last parameter
    pTempLine[0] = 0;

    return ParseParamsForOneSession(0,0);
}

mfxStatus CmdProcessor::ParseParamsForOneSession(int i,int n)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxStatus stsExtBuf = MFX_ERR_NONE;
	
	mfxU32 skipped = 0;

	TranscodingSample::sInputParams InputParams;
	if(i==0)
	{	if (m_nTimeout)
		InputParams.nTimeout = m_nTimeout;

	InputParams.shouldUseGreedyFormula = shouldUseGreedyFormula;

	InputParams.statisticsWindowSize = statisticsWindowSize;

	if (decodebuf[n]->weith == 1920)
	{
		InputParams.DecodeId = MFX_CODEC_AVC;
		InputParams.nVppCompDstX = 0;
		InputParams.nVppCompDstY = 0;
		InputParams.nVppCompDstW = 1280;
		InputParams.nVppCompDstH = 720;
		InputParams.eModeExt = VppComp;
	}
	else
	{
		InputParams.DecodeId = MFX_CODEC_MPEG2;
		InputParams.eModeExt = VppComp;
		InputParams.nVppCompDstX = 0;
		InputParams.nVppCompDstY = 0;
		InputParams.nVppCompDstW = 720;
		InputParams.nVppCompDstH = 576;
	}
	// default implementation
	InputParams.libType = MFX_IMPL_HARDWARE_ANY;
	InputParams.bUseOpaqueMemory = true;


	InputParams.dFrameRate = 25;
	InputParams.eMode = Sink;
	InputParams.bIsJoin = true;
	InputParams.nAsyncDepth = 1;
	}
	else if (i == 1)
	{ 
		if (m_nTimeout)
			InputParams.nTimeout = m_nTimeout;

		InputParams.shouldUseGreedyFormula = shouldUseGreedyFormula;

		InputParams.statisticsWindowSize = statisticsWindowSize;
		if (decodebuf[n]->weith == 1920)
		{
			InputParams.DecodeId = MFX_CODEC_AVC;
			InputParams.nVppCompDstX = 0;
			InputParams.nVppCompDstY = 0;
			InputParams.nVppCompDstW = 1280;
			InputParams.nVppCompDstH = 200;
			InputParams.eModeExt = VppComp;
			InputParams.usebigf = true;
		}
		else
		{
			InputParams.DecodeId = MFX_CODEC_AVC;
			InputParams.eModeExt = VppComp;
			InputParams.nVppCompDstX = 0;
			InputParams.nVppCompDstY = 0;
			InputParams.nVppCompDstW = 720;
			InputParams.nVppCompDstH = 100;
			InputParams.usebigf = false;
		}

		// default implementation
		InputParams.libType = MFX_IMPL_HARDWARE_ANY;
		InputParams.bUseOpaqueMemory = true;
		

		InputParams.dFrameRate = 25;
		InputParams.eMode = Sink;
		InputParams.bIsJoin = true;
		InputParams.nAsyncDepth = 1;
	}
	else if (i == 2)
	{
		if (m_nTimeout)
			InputParams.nTimeout = m_nTimeout;

		InputParams.shouldUseGreedyFormula = shouldUseGreedyFormula;

		InputParams.statisticsWindowSize = statisticsWindowSize;

		
		// default implementation
		InputParams.libType = MFX_IMPL_HARDWARE_ANY;
		InputParams.bUseOpaqueMemory = true;
		if (decodebuf[n]->weith == 1920)
		{
			InputParams.nDstWidth = 1280;
			InputParams.nDstHeight = 720;
		}
		else
		{
			InputParams.nDstWidth = 720;
			InputParams.nDstHeight = 576;
		}
		
		InputParams.eMode = Source;
		InputParams.eModeExt = VppComp;
		InputParams.numSurf4Comp = 2;
		//InputParams.nLADepth = 40;
		InputParams.nRateControlMethod = MFX_RATECONTROL_CBR;
		InputParams.EncodeId = MFX_CODEC_AVC;
		InputParams.bEnableDeinterlacing = MFX_CODINGOPTION_ON;
		InputParams.DeinterlacingMode = MFX_DEINTERLACING_ADVANCED;
		InputParams.GopPicSize = 100;		//	GOP数组大小
		//码率
		InputParams.nBitRate = 800;
		InputParams.GopRefDist = 5;			//	P帧间隔 GOP=50:3	GOP=100:5		间隔越大，B帧越多，码率越低
		InputParams.bEnableBPyramid = true;
		InputParams.bIsJoin = true;
		InputParams.nAsyncDepth = 1;
	}

	
		sts = VerifyAndCorrectInputParams(InputParams);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		m_SessionArray.push_back(InputParams);
	

	return MFX_ERR_NONE;
		/*	while (decodebuf[j]->weith == 0)
				Sleep(100);
		if (m_nTimeout)
			InputParams.nTimeout = m_nTimeout;
		InputParams.shouldUseGreedyFormula = shouldUseGreedyFormula;
		InputParams.statisticsWindowSize = statisticsWindowSize;
		InputParams.libType = MFX_IMPL_HARDWARE_ANY;
		InputParams.bUseOpaqueMemory = true;
		InputParams.eModeExt = Native;
		InputParams.EncodeId = MFX_CODEC_AVC;
		InputParams.dFrameRate = 25;
		InputParams.nTargetUsage = 7;
		InputParams.nAsyncDepth = 1;
		InputParams.GopPicSize = 50;
		InputParams.GopRefDist = 4;
		InputParams.nGpuCopyMode = MFX_GPUCOPY_ON;
		InputParams.bEnableDeinterlacing = true;
		InputParams.DeinterlacingMode = MFX_DEINTERLACING_ADVANCED;
		InputParams.nLADepth = 40;
		InputParams.nRateControlMethod = MFX_RATECONTROL_LA_ICQ;
		/*InputParams.nRotationAngle = 180;
		//InputParams.bOpenCL = true;
		const char *s = "sample_rotate_plugin.dll";
		for (int i = 0; i < 24; i++)
			InputParams.strVPPPluginDLLPath[i] = *(s + i);*/
			/*if (decodebuf[j]->weith == 1920)
			{
				InputParams.nDstWidth = 1920;
				InputParams.nDstHeight = 1080;
				InputParams.DecodeId = MFX_CODEC_AVC;
			}
			else if (decodebuf[j]->weith == 720)
			{
				InputParams.nDstWidth = 720;
				InputParams.nDstHeight = 576;
				InputParams.DecodeId = MFX_CODEC_MPEG2;
			}
			else if (decodebuf[j]->weith == 704)
			{
				InputParams.nDstWidth = 704;
				InputParams.nDstHeight = 576;
				InputParams.DecodeId = MFX_CODEC_MPEG2;
			}*/
		
 
} //mfxStatus CmdProcessor::ParseParamsForOneSession(msdk_char *pLine, mfxU32 length)

mfxStatus CmdProcessor::ParseOption__set(msdk_char* strCodecType, msdk_char* strPluginPath)
{
    mfxU32 codecid = 0;
    mfxU32 type = 0;
    sPluginParams pluginParams;

    //Parse codec type - decoder or encoder
    if (0 == msdk_strncmp(MSDK_STRING("-i::"), strCodecType, 4))
    {
        type = MSDK_VDECODE;
    }
    else if (0 == msdk_strncmp(MSDK_STRING("-o::"), strCodecType, 4))
    {
        type = MSDK_VENCODE;
    }
    else
    {
        msdk_printf(MSDK_STRING("error: incorrect definition codec type (must be -i:: or -o::)\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (StrFormatToCodecFormatFourCC(strCodecType+4, codecid) != MFX_ERR_NONE)
    {
        msdk_printf(MSDK_STRING("error: codec is unknown\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!IsPluginCodecSupported(codecid))
    {
        msdk_printf(MSDK_STRING("error: codec is unsupported\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    pluginParams = ParsePluginParameter(strPluginPath);

    if (type == MSDK_VDECODE)
        m_decoderPlugins.insert(std::pair<mfxU32, sPluginParams>(codecid, pluginParams));
    else
        m_encoderPlugins.insert(std::pair<mfxU32, sPluginParams>(codecid, pluginParams));

    return MFX_ERR_NONE;
};

sPluginParams CmdProcessor::ParsePluginParameter(msdk_char* strPluginPath)
{
    sPluginParams pluginParams;
    if (MFX_ERR_NONE == ConvertStringToGuid(strPluginPath, pluginParams.pluginGuid))
    {
        pluginParams.type = MFX_PLUGINLOAD_TYPE_GUID;
    }
    else
    {
#if defined(_WIN32) || defined(_WIN64)
        msdk_char wchar[1024];
        msdk_opt_read(strPluginPath, wchar);
        std::wstring wstr(wchar);
        std::string str(wstr.begin(), wstr.end());

        strcpy_s(pluginParams.strPluginPath, str.c_str());
#else
        msdk_opt_read(strPluginPath, pluginParams.strPluginPath);
#endif
        pluginParams.type = MFX_PLUGINLOAD_TYPE_FILE;
    }

    return pluginParams;
}


mfxStatus CmdProcessor::VerifyAndCorrectInputParams(TranscodingSample::sInputParams &InputParams)
{
    

    if (MFX_CODEC_JPEG != InputParams.EncodeId && MFX_CODEC_MPEG2 != InputParams.EncodeId &&
        MFX_CODEC_AVC != InputParams.EncodeId && MFX_CODEC_HEVC != InputParams.EncodeId &&
        MFX_CODEC_VP8 != InputParams.EncodeId && MFX_FOURCC_DUMP != InputParams.EncodeId &&
        InputParams.eMode != Sink && InputParams.eModeExt != VppCompOnly)
    {
        PrintError(MSDK_STRING("Unknown encoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != InputParams.DecodeId &&
       MFX_CODEC_AVC != InputParams.DecodeId &&
       MFX_CODEC_HEVC != InputParams.DecodeId &&
       MFX_CODEC_VC1 != InputParams.DecodeId &&
       MFX_CODEC_JPEG != InputParams.DecodeId &&
       MFX_CODEC_VP8 != InputParams.DecodeId &&
       InputParams.eMode != Source)
    {
        PrintError(MSDK_STRING("Unknown decoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if(InputParams.dEncoderFrameRate && InputParams.bEnableExtLA)
    {
        PrintError(MSDK_STRING("-la_ext and -fe options cannot be used together\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.nQuality && InputParams.EncodeId && (MFX_CODEC_JPEG != InputParams.EncodeId))
    {
        PrintError(MSDK_STRING("-q option is supported only for JPEG encoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((InputParams.nTargetUsage || InputParams.nBitRate) && (MFX_CODEC_JPEG == InputParams.EncodeId))
    {
        PrintError(MSDK_STRING("-b and -u options are supported only for H.264, MPEG2 and MVC encoders. For JPEG encoder use -q\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    // valid target usage range is: [MFX_TARGETUSAGE_BEST_QUALITY .. MFX_TARGETUSAGE_BEST_SPEED] (at the moment [1..7])
    if ((InputParams.nTargetUsage < MFX_TARGETUSAGE_BEST_QUALITY) ||
        (InputParams.nTargetUsage > MFX_TARGETUSAGE_BEST_SPEED) )
    {
        if (InputParams.nTargetUsage == MFX_TARGETUSAGE_UNKNOWN)
        {
            // if user did not specified target usage - use balanced
            InputParams.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
        }
        else
        {
            PrintError(NULL,"Unsupported target usage");
            // report error if unsupported target usage was set
            return MFX_ERR_UNSUPPORTED;
        }
    }

    // Ignoring user-defined Async Depth for LA
    if (InputParams.nMaxSliceSize)
    {
        InputParams.nAsyncDepth = 1;
    }

    if (InputParams.bLABRC && !(InputParams.libType & MFX_IMPL_HARDWARE_ANY))
    {
        PrintError(MSDK_STRING("Look ahead BRC is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.bLABRC && (InputParams.EncodeId != MFX_CODEC_AVC) && (InputParams.eMode == Source))
    {
        PrintError(MSDK_STRING("Look ahead BRC is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.nLADepth && (InputParams.nLADepth < 10 || InputParams.nLADepth > 100))
    {
        if ((InputParams.nLADepth != 1) || (!InputParams.nMaxSliceSize))
        {
            PrintError(MSDK_STRING("Unsupported value of -lad parameter, must be in range [10, 100] or 1 in case of -mss option!"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    if (InputParams.nRateControlMethod == 0)
    {
        InputParams.nRateControlMethod = MFX_RATECONTROL_CBR;
    }

    if ((InputParams.nMaxSliceSize) && !(InputParams.libType & MFX_IMPL_HARDWARE_ANY))
    {
        PrintError(MSDK_STRING("MaxSliceSize option is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }
    if ((InputParams.nMaxSliceSize) && (InputParams.nSlices))
    {
        PrintError(MSDK_STRING("-mss and -l options are not compatible!"));
    }
    if ((InputParams.nMaxSliceSize) && (InputParams.EncodeId != MFX_CODEC_AVC))
    {
        PrintError(MSDK_STRING("MaxSliceSize option is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if(InputParams.enableQSVFF && InputParams.eMode == Sink)
    {
        msdk_printf(MSDK_STRING("WARNING: -qsv-ff option is not valid for decoder-only sessions, this parameter will be ignored.\n"));
    }

    std::map<mfxU32, sPluginParams>::iterator it;

    // Set decoder plugins parameters only if they were not set before
    if (!memcmp(InputParams.decoderPluginParams.pluginGuid.Data, MSDK_PLUGINGUID_NULL.Data, sizeof(MSDK_PLUGINGUID_NULL)) &&
         !strcmp(InputParams.decoderPluginParams.strPluginPath,""))
    {
        it = m_decoderPlugins.find(InputParams.DecodeId);
        if (it != m_decoderPlugins.end())
            InputParams.decoderPluginParams = it->second;
    }
    else
    {
        // Decoding plugin was set manually, so let's check if codec supports plugins
        if (!IsPluginCodecSupported(InputParams.DecodeId))
        {
            msdk_printf(MSDK_STRING("error: decoder does not support plugins\n"));
            return MFX_ERR_UNSUPPORTED;
        }

    }

    // Set encoder plugins parameters only if they were not set before
    if (!memcmp(InputParams.encoderPluginParams.pluginGuid.Data, MSDK_PLUGINGUID_NULL.Data, sizeof(MSDK_PLUGINGUID_NULL)) &&
        !strcmp(InputParams.encoderPluginParams.strPluginPath, ""))
    {
        it = m_encoderPlugins.find(InputParams.EncodeId);
        if (it != m_encoderPlugins.end())
            InputParams.encoderPluginParams = it->second;
    }
    else
    {
        // Decoding plugin was set manually, so let's check if codec supports plugins
        if (!IsPluginCodecSupported(InputParams.DecodeId))
        {
            msdk_printf(MSDK_STRING("error: encoder does not support plugins\n"));
            return MFX_ERR_UNSUPPORTED;
        }

    }

    if(InputParams.EncoderFourCC && InputParams.eMode == Sink)
    {
        msdk_printf(MSDK_STRING("WARNING: -ec option is used in session without encoder, this parameter will be ignored \n"));
    }

    if(InputParams.DecoderFourCC && InputParams.eMode != Native && InputParams.eMode != Sink)
    {
        msdk_printf(MSDK_STRING("WARNING: -dc option is used in session without decoder, this parameter will be ignored \n"));
    }

    if(InputParams.FRCAlgorithm && InputParams.eMode == Sink)
    {
        PrintError(NULL,MSDK_STRING("-FRC option should not be used in -o::sink pipelines \n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if(InputParams.EncoderFourCC && InputParams.EncoderFourCC != MFX_FOURCC_NV12 &&
        InputParams.EncoderFourCC != MFX_FOURCC_RGB4 && InputParams.EncoderFourCC != MFX_FOURCC_YUY2 &&
        InputParams.EncodeId == MFX_FOURCC_DUMP)
    {
        PrintError(MSDK_STRING("-o::raw option can be used with NV12, RGB4 and YUY2 color formats only.\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
} //mfxStatus CmdProcessor::VerifyAndCorrectInputParams(TranscodingSample::sInputParams &InputParams)

bool  CmdProcessor::GetNextSessionParams(TranscodingSample::sInputParams &InputParams)
{
    if (!m_SessionArray.size())
        return false;
    if (m_SessionParamId == m_SessionArray.size())
    {
        return false;
    }
    InputParams = m_SessionArray[m_SessionParamId];

    m_SessionParamId++;
    return true;

} //bool  CmdProcessor::GetNextSessionParams(TranscodingSample::sInputParams &InputParams)

mfxU16 TranscodingSample::FourCCToChroma(mfxU32 fourCC)
{
    switch(fourCC)
    {
    case MFX_FOURCC_NV12:
    case MFX_FOURCC_P010:
        return MFX_CHROMAFORMAT_YUV420;
    case MFX_FOURCC_NV16:
    case MFX_FOURCC_P210:
    case MFX_FOURCC_YUY2:
        return MFX_CHROMAFORMAT_YUV422;
    case MFX_FOURCC_RGB4:
        return MFX_CHROMAFORMAT_YUV444;
    }

    return MFX_CHROMAFORMAT_YUV420;
}
