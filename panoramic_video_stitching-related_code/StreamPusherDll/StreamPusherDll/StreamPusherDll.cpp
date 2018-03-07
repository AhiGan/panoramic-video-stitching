// StreamPusherDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "StreamPusherDll.h"


extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"

#include "libavutil/audio_fifo.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
	//#include "SDL/SDL.h"

#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};



// ���ǵ���������һ��ʾ��
STREAMPUSHERDLL_API int nStreamPusherDll=0;

// ���ǵ���������һ��ʾ����
STREAMPUSHERDLL_API int fnStreamPusherDll(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� StreamPusherDll.h
CStreamPusherDll::CStreamPusherDll()
{
	return;
}


typedef unsigned char* PUCHAR;
extern "C"
{
	FILE  *fp_yuv;			//�������ͷ����yuv��ָ���ļ�


	//��Ƶ����
	AVFormatContext* pFormatCtx;   //��Ƶ����
	AVOutputFormat* fmt;			//��Ƶ���������ʽ
	AVStream* video_st;				//��Ƶ������
	AVCodecContext* pCodecCtx;		//��Ƶ������������
	AVCodec* pCodec;				//��Ƶ������
	AVPacket pkt;					//�����һ֡����
	int looptime;					//���ڿ��Ʊ��뵽�ļ���h.264��֡�� �Լ�����ÿ֡��ptsֵ
	bool IsEncodeFrame;				//�����ж��Ƿ����һ֡��Ƶ���ݴ�YUV->H.264

	//��װʱ��ǰ��Ƶ/��Ƶ֡��pts�����ڱȽ��ж�д����Ƶ������Ƶ֡
	int cur_pts_v;
	//����
	AVOutputFormat *ofmt;			//��������ʽ
	AVFormatContext  *ofmt_ctx;	//Output AVFormatContext
	int Ret;    //���ڷ������������е�һЩ��������

	void freeAllocateVariable(){
		/* close output */
		if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
			avio_close(ofmt_ctx->pb);
		avformat_free_context(ofmt_ctx);
		if (Ret < 0 && Ret != AVERROR_EOF) {
			printf("freeAllocateVariable:Error occurred.\n");
			//return -1;
		}
	}
	void encodeInit(int m_nWidth, int m_nHeight){
		int in_w = m_nWidth, in_h = m_nHeight;                              //Input data's width and height
		looptime = 0;
		cur_pts_v = -1;
		const char* out_file = "ds.h264";
		av_register_all();//ע���������
		//Method1.
		pFormatCtx = avformat_alloc_context();//�����װ��ʽ�����Ľṹ���󣬸ýṹ������Ƶ��װ��ʽ�����Ϣ
		//Guess Format
		fmt = av_guess_format(NULL, out_file, NULL);//���������ʽ Ϊ����ļ�ָ���ĸ�ʽ
		pFormatCtx->oformat = fmt;//��pFormatCtx���container��ʽ����Ϊ�ļ���ds.h264��
		//Method 2.
		//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
		//fmt = pFormatCtx->oformat;

		//Open output URL  ���ļ���Ƶ��Դ
		if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0){
			printf("EncodeInit:Failed to open output file! \n");
		}
		video_st = avformat_new_stream(pFormatCtx, 0);//ΪpFormatCtx���һ���µ��������stream
		video_st->time_base.num = 1;
		video_st->time_base.den = 2;
		video_st->r_frame_rate.num = 2;//����video_st��֡��
		video_st->r_frame_rate.den = 1;
		video_st->duration = 1;//��֡ͼƬ֮��ļ��
		if (video_st == NULL){
			printf("EncodeInit:init video_st failed");
		}
		//Param that must set
		pCodecCtx = video_st->codec;
		//pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
		pCodecCtx->codec_id = fmt->video_codec;//���ñ����ʽΪh.264
		pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;//���ñ�������Ϊ��Ƶ����
		pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//������Ҫ����ԭʼͼ��֡��ʽ
		pCodecCtx->width = in_w;  //����ͼ��� ��
		pCodecCtx->height = in_h;
		pCodecCtx->time_base.num = 1;
		pCodecCtx->time_base.den = 2;
		pCodecCtx->bit_rate = 400000;
		pCodecCtx->gop_size = 10;  //���������ü�����һ���ؼ�֡
		//H264
		//pCodecCtx->me_range = 16;
		//pCodecCtx->max_qdiff = 4;
		//pCodecCtx->qcompress = 0.6;
		pCodecCtx->qmin = 10;//��Ƶ�������ֵ
		pCodecCtx->qmax = 51;//��Ƶ�������ֵ

		//Optional Param
		//pCodecCtx->max_b_frames=3;

		// Set Option
		AVDictionary *param = 0;
		//H.264
		if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
			av_dict_set(&param, "preset", "slow", 0);//����AVDictionary
			av_dict_set(&param, "tune", "zerolatency", 0);
			//av_dict_set(&param, "profile", "main", 0);
		}
		//H.265
		if (pCodecCtx->codec_id == AV_CODEC_ID_H265){
			av_dict_set(&param, "preset", "ultrafast", 0);
			av_dict_set(&param, "tune", "zero-latency", 0);
		}

		//Show some Information
		av_dump_format(pFormatCtx, 0, out_file, 1);

		pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
		if (!pCodec){
			printf("EncodeInit:Can not find encoder! \n");
			//return -1;
		}
		if (avcodec_open2(pCodecCtx, pCodec, &param) < 0){
			printf("EncodeInit:Failed to open encoder! \n");
			//return -1;
		}

		//Write File Header
		avformat_write_header(pFormatCtx, NULL);
	}
	void streamInit(const char* pushAddress )
	{
		ofmt = NULL;
		ofmt_ctx = NULL;
		//AVPacket pkt;
		//const char *in_filename, *out_filename;
		char out_filename[50];
		//int i;
		//int videoindex=-1;
		//int frame_index=0;

		//in_filename  = "cuc_ieschool.h264";

		strcpy(out_filename, pushAddress);
		//out_filename = "rtmp://223.3.87.34:1935/live1/room1";
		//out_filename = "rtmp://192.168.1.104:1935/live1/room1";
		//out_filename = "rtmp://push.gz.bcelive.com/live/asmety0c5iaxd9ls4j";
		//out_filename = "rtmp://push.gz.bcelive.com/live/goifqjyfvma0lz6czc";
		//out_filename = "rtmp://live1.sinacloud.com/publish/72bd759e89be93bd6619fcba19d486f5";
		//out_filename = "fuquan.flv";
		av_register_all();
		//Network
		avformat_network_init();

		//Output
		avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename); //RTMP
		//avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", out_filename);//UDP

		if (!ofmt_ctx) {
			printf("streamInit:Could not create output context\n");
			Ret = AVERROR_UNKNOWN;
			freeAllocateVariable();
		}
		ofmt = ofmt_ctx->oformat;

		//Create output AVStream according to input AVStream
		//AVStream *in_stream = video_st;
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, video_st->codec->codec);
		if (!out_stream) {
			printf("streamInit:Failed allocating output stream\n");
			Ret = AVERROR_UNKNOWN;
			freeAllocateVariable();
		}
		//Copy the settings of AVCodecContext
		Ret = avcodec_copy_context(out_stream->codec, video_st->codec);
		if (Ret < 0) {
			printf("streamInit:Failed to copy context from input to output stream codec context\n");
			freeAllocateVariable();
		}
		out_stream->codec->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

		//Dump Format------------------
		av_dump_format(ofmt_ctx, 0, out_filename, 1);
		//Open output URL
		if (!(ofmt->flags & AVFMT_NOFILE)) {
			Ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
			if (Ret < 0) {
				printf("streamInit:Could not open output URL '%s'", out_filename);
				freeAllocateVariable();
			}
		}
		//Write file header
		Ret = avformat_write_header(ofmt_ctx, NULL);
		if (Ret < 0) {
			printf("streamInit:Error occurred when opening output URL\n");
			freeAllocateVariable();
		}
	}
	unsigned char* FrameRGBtoYUV(int Fheight, int Fwidth, PUCHAR tpData, AVFrame*  pFrameYUV)
	{
		unsigned char *ptrY, *ptrU, *ptrV;
		unsigned char *ptrRGB;
		unsigned char *YUV420Buffer = (unsigned char *)malloc(Fwidth*Fheight * 3 / 2);
		memset(YUV420Buffer, 0, Fwidth*Fheight * 3 / 2);//����һ������ָ����ڴ��еĵ����������������ֽ�����Ϊ�ڶ�������ֵ��
		ptrY = YUV420Buffer;
		ptrU = YUV420Buffer + Fheight*Fwidth;
		ptrV = ptrU + Fheight*Fwidth * 1 / 4;
		unsigned char y, u, v, r, g, b;
		//RGB--->YUV
		for (int j = 0; j< Fheight; j++)
		{
			ptrRGB = tpData + j*Fwidth * 3;
			for (int i = 0; i < Fwidth; i++)
			{
				r = *(ptrRGB++);
				g = *(ptrRGB++);
				b = *(ptrRGB++);
				y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
				u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
				v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
				if (y<0 || y>255)
				{
					if (y<0)
						*(ptrY++) = 0;
					else
						*(ptrY++) = 255;
				}
				else
				{
					*(ptrY++) = y;
				}
				if (j % 2 == 0 && i % 2 == 0)
				{
					if (u<0 || u>255)
					{
						if (u<0)
							*(ptrU++) = 0;
						else
							*(ptrU++) = 255;
					}
					else
					{
						*(ptrU++) = u;
					}
				}
				else
				{
					if (i % 2 == 0)
					{
						if (v<0 || v>255)
						{
							if (v<0)
								*(ptrV++) = 0;
							else
								*(ptrV++) = 255;
						}
						else
						{
							*(ptrV++) = v;
						}
					}
				}

			}
		}
		/*
		uint8_t *data[AV_NUM_DATA_POINTERS]�������ԭʼ���ݣ�����Ƶ��˵��YUV��RGB������Ƶ��˵��PCM��
		int linesize[AV_NUM_DATA_POINTERS]��data�С�һ�С����ݵĴ�С��ע�⣺δ�ص���ͼ��Ŀ�һ�����ͼ��Ŀ�
		����packed��ʽ�����ݣ�����RGB24������浽data[0]���档
		����planar��ʽ�����ݣ�����YUV420P�������ֿ���data[0]
		��data[1]��data[2]...��YUV420P��data[0]��Y��data[1]��U��data[2]��V��
		*/
		pFrameYUV->data[0] = YUV420Buffer;		//����Y��U��V����ƽ�������
		pFrameYUV->data[1] = YUV420Buffer + Fwidth*Fheight;
		pFrameYUV->data[2] = YUV420Buffer + 5 / 4 * Fheight*Fwidth;
		pFrameYUV->linesize[0] = Fwidth;
		pFrameYUV->linesize[1] = Fwidth / 2;
		pFrameYUV->linesize[2] = Fwidth / 2;

		pFrameYUV->width = Fwidth;
		pFrameYUV->height = Fheight;

		return YUV420Buffer;
	}
	bool YUVencodeToH264(AVFrame* pFrameYUV)
	{
		av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;
		pFrameYUV->pts = looptime++;
		int got_picture = 0;
		//Encode
		int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrameYUV, &got_picture);
		if (ret < 0){
			printf("YUVencodeToH264:Failed to encode! \n");
			//return -1;
		}
		if (got_picture == 1){
			printf("Succeed to encode frame: %5d\tsize:%5d\n", looptime, pkt.size);
			//framecnt++;
			pkt.stream_index = video_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			if (ret < 0)
			{
				printf("fail to write a pkt frame to out file ");
			}
			/*
			//�Ի�ȡ��ѹ����Ƶh.264��pkt ���з�װ д�뵽��װ����
			if(pkt.pts==AV_NOPTS_VALUE){
			//Write PTS
			AVRational time_base1=video_st->time_base;
			//Duration between 2 frames (us)
			int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(video_st->r_frame_rate);
			//Parameters
			pkt.pts=(double)((looptime-1)*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts=pkt.pts;
			pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
			//frame_index++;
			}
			//Convert PTS/DTS
			pkt.pts = av_rescale_q_rnd(pkt.pts, video_st->time_base, ofmt_ctx_muxer->streams[videoindex_out]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, video_st->time_base, ofmt_ctx_muxer->streams[videoindex_out]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, video_st->time_base, ofmt_ctx_muxer->streams[videoindex_out]->time_base);
			pkt.pos = -1;
			printf("Write 1 Packet. size:%5d\tpts:%lld\n",pkt.size,pkt.pts);
			//Write
			if (av_interleaved_write_frame(ofmt_ctx_muxer, &pkt) < 0) {//����1�������pkt�е�dts��ptsΪʲô�ı���
			printf( "Error muxing packet\n");
			//break;
			}
			av_free_packet(&pkt);//�����������������streamPush���ͷ�pkt
			*/
			return true;
		}
		else {
			return false;
		}
	}
	void streamPush(){
		AVStream *in_stream, *out_stream;
		//FIX��No PTS (Example: Raw H.264)
		//Simple Write PTS
		if (pkt.pts == AV_NOPTS_VALUE){ //��Ϊǰ��������pkt��pts���������ﲻ��ִ��������� ������dts��pts��ͬ����Ϊpktѹ��ֻ֡��P��I֡������ʹ����ȣ������B֡�Ͳ�����
			//Write PTS
			AVRational time_base1 = video_st->time_base;
			//Duration between 2 frames (us)
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(video_st->r_frame_rate);
			//Parameters
			pkt.pts = (double)((looptime - 1)*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		}
		in_stream = video_st;
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		/* copy packet */
		//Convert PTS/DTS �������AVStreamʱ��ת�������AVStream��ʱ�����޸�����Ӧ��pts��dts�Լ�duration
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//Print to Screen
		/*
		if(pkt.stream_index==videoindex){
		printf("Send %8d video frames to output URL\n",frame_index);
		frame_index++;
		}
		*/
		//Ret = av_write_frame(ofmt_ctx, &pkt);
		Ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (Ret < 0) {
			printf("streamPush:Error muxing packet\n");
			//break;
		}
	}
	int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index){

		int ret;
		int got_frame;
		AVPacket enc_pkt;
		if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
			CODEC_CAP_DELAY))
			return 0;
		while (1) {
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			av_init_packet(&enc_pkt);
			ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
				NULL, &got_frame);
			av_frame_free(NULL);
			if (ret < 0)
				break;
			if (!got_frame){
				ret = 0;
				break;
			}
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
			// mux encoded frame 
			ret = av_write_frame(fmt_ctx, &enc_pkt);
			if (ret < 0)
				break;
			else
			{

				//����
				AVStream *in_stream, *out_stream;

				//FIX��No PTS (Example: Raw H.264)
				//Simple Write PTS
				if (enc_pkt.pts == AV_NOPTS_VALUE){
					//Write PTS
					AVRational time_base1 = video_st->time_base;
					//Duration between 2 frames (us)
					int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(video_st->r_frame_rate);
					//Parameters
					enc_pkt.pts = (double)(looptime*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
					enc_pkt.dts = pkt.pts;
					enc_pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
				}



				in_stream = video_st;
				out_stream = ofmt_ctx->streams[pkt.stream_index];
				// copy packet 
				//Convert PTS/DTS
				enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				enc_pkt.duration = av_rescale_q(enc_pkt.duration, in_stream->time_base, out_stream->time_base);
				enc_pkt.pos = -1;
				//Print to Screen
				/*
				if(pkt.stream_index==videoindex){
				printf("Send %8d video frames to output URL\n",frame_index);
				frame_index++;
				}
				*/

				ret = av_write_frame(ofmt_ctx, &pkt);
				//ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);

				if (ret < 0) {
					printf("flush_encoder:Error muxing packet\n");
					//break;
				}

				av_free_packet(&enc_pkt);

			}
			av_free_packet(&enc_pkt);
		}
		return ret;
	}
	void STREAMPUSHERDLL_API encodeAndPushInit(int Fwidth, int Fheight, const char* pushAddress)
	{
		fp_yuv = fopen("output.yuv", "wb+");
		encodeInit(Fwidth, Fheight);
		streamInit(pushAddress);
	}
	void STREAMPUSHERDLL_API encodeAndPush(int Fwidth, int Fheight, PUCHAR tpData)
	{
		AVFrame	*pFrameYUV;//AVFrame�ṹ��һ�����ڴ洢ԭʼ���ݣ�����ѹ�����ݣ��������Ƶ��˵��YUV��RGB������Ƶ��˵��PCM�������⻹������һЩ��ص���Ϣ��
		pFrameYUV = av_frame_alloc();
		//RGB---->YUVת��
		unsigned char* YUV420Buffer = FrameRGBtoYUV(Fheight, Fwidth, tpData, pFrameYUV);
		//д��YUV���ļ���
		
		int y_size= Fwidth*Fheight;
		fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
		fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
		fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
		
		//����
		bool IsEncodeFrame = YUVencodeToH264(pFrameYUV);
		if (IsEncodeFrame)
		{
			//����
			streamPush();
			av_free_packet(&pkt);
		}
		//�ͷŴ���YUVʹ�õ�֡����
		av_free(pFrameYUV);
		free(YUV420Buffer);
	}
	void STREAMPUSHERDLL_API pushEnd()
	{
		//�رջص������д򿪵����ڴ洢yuv��Ƶ���ļ�
		fclose(fp_yuv);
		//��Ƶ�������
		int ret = flush_encoder(pFormatCtx, 0);
		if (ret < 0) {
			printf("Flushing encoder failed\n");
			//return -1;
		}
		//Write file trailer
		av_write_trailer(pFormatCtx);

		//Clean
		if (video_st){
			avcodec_close(video_st->codec);
			//av_free(pFrameYUV);
		}
		avio_close(pFormatCtx->pb);
		avformat_free_context(pFormatCtx);
		//��������
		//Write file trailer

		av_write_trailer(ofmt_ctx);
		freeAllocateVariable();
	}

};
