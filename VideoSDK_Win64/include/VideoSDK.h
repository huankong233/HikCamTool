#ifndef HIK_VIDEOSDK_H
#define HIK_VIDEOSDK_H

#if (defined(WIN32) || defined(WIN64))
#if defined(VIDEOSDK_EXPORTS)
#define VIDEOSDK_DECLARE extern "C" __declspec(dllexport)
#else
#define VIDEOSDK_DECLARE extern "C" __declspec(dllimport)
#endif // defined(VIDEOSDK_EXPORTS)

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#else
#define VIDEOSDK_DECLARE extern "C"
#ifndef CALLBACK
#define CALLBACK
#endif
#endif  // (defined(WIN32) || defined(WIN64))

#ifndef _IN_
#define _IN_            // 表示入参
#endif

#ifndef _OUT_
#define _OUT_           // 表示出参
#endif

typedef long long VIDEO_INT64;  // 统一8字节数据类型

                                ////////////////////////// 错误码定义 ////////////////////////////////
#define VIDEO_ERR_FAIL            -1     // 失败
#define VIDEO_ERR_SUCCESS          0     // 成功
#define VIDEO_ERR_PARAM            1     // 参数错误
#define VIDEO_ERR_INVALID_POINTER  2     // 空指针
#define VIDEO_ERR_NOT_INIT         3     // 未初始化
#define VIDEO_ERR_INVALID_HANDLE   4     // 句柄无效
#define VIDEO_ERR_CREATE_INSTANCE  5     // 创建实例失败
#define VIDEO_ERR_APPLY_MEMORY     6     // 申请内存失败
#define VIDEO_ERR_NET_EXCEPTION    7     // 网络异常
#define VIDEO_ERR_NOT_SUPPORT      8     // 不支持，如只取流时抓图，rtp流查询当前播放时间戳
#define VIDEO_ERR_SET_CALLBACK     9     // 回调设置失败
#define VIDEO_ERR_START_STREAM     10    // 取流失败
#define VIDEO_ERR_STOP_STREAM      11    // 停止取流异常
#define VIDEO_ERR_START_PLAY       12    // 解码失败
#define VIDEO_ERR_STOP_PLAY        13    // 停止解码异常
#define VIDEO_ERR_TIMESTAMP        14    // 时间戳无效
#define VIDEO_ERR_SOUND            15    // 声音相关错误
#define VIDEO_ERR_SNAP             16    // 抓拍失败
#define VIDEO_ERR_GETPLAYTIME      17    // 查询播放时间戳失败
#define VIDEO_ERR_INNER            18    // 内部错误
#define VIDEO_ERR_PLAYCTRL         19    // 播放、下载控制失败
#define VIDEO_ERR_OPER_ORDER       20    // 操作次序不对（如未打开声音就获取音量或设置音量）
#define VIDEO_ERR_DISKSPACE        21    // 磁盘空间不足
#define VIDEO_ERR_LOADLIBARAY      22    // 模块加载失败，一般指萤石进程相关的dll由于缺库等原因无法加载
#define VIDEO_ERR_VIDEOINFO        23    // 查询视频详情失败
#define VIDEO_ERR_CREATEFILE       24    // 创建文目录或件失败，一般是路径、文件名称含有非法字符，如路径中目录名称全是空格字符串，中英文下的“*”、“|”以及英文下的“?”
#define VIDEO_ERR_OSD              25    // 画面字符叠加错误
#define VIDEO_ERR_TALK             26    // 对讲相关错误
#define VIDEO_ERR_DEVICE           27    // 设备相关错误，如对讲无法检测到声卡设备或声音采集设备
#define VIDEO_ERR_ALLOC_RESOURCE   28    // 分配资源错误
#define VIDEO_ERR_PRIVATEDATA_CTRL 29    // 私有数据控制失败
#define VIDEO_ERR_MAX_ABILITY      30    // 超过最大处理能力，如录像下载或录像需要转封装时达到转封装路数上限
#define VIDEO_ERR_STREAM_TRANSFORM 31    // 流数据转换错误
#define VIDEO_ERR_FILE_INIVALID    32    // 无效文件（断点续传时，分析文件发现不符合断点续传要求时会报此错误码，一般报此错误码表示文件中数据不足，不足以分析断点续传信息）
#define VIDEO_ERR_TIMESTAMP_NOT_IN_RANGE 33  // 录像下载时指定断点续传，但分析出断点处时间不在指定的开始与结束时间之间
#define VIDEO_ERR_INSTANSPLAY      34    // 即时回放相关错误
#define VIDEO_ERR_EAGLEEYE         35    // 鹰眼相关错误
#define VIDEO_ERR_CONFIG           36    // 配置文件配置项错误


#define VIDEO_ERR_DIGITALZOOM      100   // 电子放大相关错误
#define VIDEO_ERR_EZVIZ_NOT_INIT   300   // 萤石未初始化
#define VIDEO_ERR_FILE_NOT_EXIST   500   // 文件不存在，一般指萤石进程相关的dll或exe
#define VIDEO_ERR_TIMEOUT          501   // 请求超时，一般指直连萤石进程通信的超时
#define VIDEO_ERR_PROCESS_STARTUP  502   // 萤石进程启动失败
#define VIDEO_ERR_EZVIZ_RELATIVE_PATH  502   //配置文件中萤石进程相对VIdeoSDK.dll路径配置错误
#define VIDEO_ERR_PROCESSFRAMEWORK 600   // 进程框架相关错误
#define VIDEO_ERR_EZVIZ            700   // 萤石业务层相关错误
#define VIDEO_ERR_OTHER            10000 // 其它错误


////////////////////////// 回调定义 ///////////////////////////////
// 码流回调
// i64PlayHandle：预览或回放接口返回的句柄
// iStreamDataType：码流数据类型，0-码流头 1-码流数据 2-结束标记
// pDataArray：流数据数组（需当成二进制数据来解析）
// iDataLen：流数据长度
// pUserData：用户数据
typedef void(CALLBACK * pfnStreamCallback)(VIDEO_INT64 i64PlayHandle, int iStreamDataType, const char* pDataArray, int iDataLen, void* pUserData);

// 取流、播放消息、萤石语音对讲消息回调
// i64PlayHandle：预览或回放接口返回的句柄，当回调的是语音对讲取流消息时，该值无效值（-1）
// iMsg：消息类型，1-播放开始 2-播放结束（视频预览无此消息） 3-播放异常 4-播放加密密钥错误 10-取流开始 11-取流结束 12-取流异常 13-倒放取流不支持 30-即时回放开始解码 31-即时回放解码结束（正常播放结束时才有，停止即时回放不会回调此消息） 50-异步时发起取流请求成功 51-异步时发起取流请求失败 70-萤石开始语音对讲成功消息 71-萤石停止语音对讲成功消息72-萤石语音对讲异常消息
typedef void(CALLBACK * pfnMsgCallback)(VIDEO_INT64 i64PlayHandle, int iMsg, void* pUserData);

// 绘图回调
// i64PlayHandle：预览或回放接口返回的句柄
// hDC：HDC
// pUserData：用户数据
// iDrawType：绘图类型，0-普通绘图回调 1-电子放大时全景窗口绘图回调
typedef void (CALLBACK* DrawCallback)(VIDEO_INT64 i64PlayHandle, void* hDC, void* pUserData, int iDrawType);

// YUV数据回调
// i64PlayHandle：预览或回放接口返回的句柄
// pDataArray：YUV数据数组（需当成二进制数据来解析）
// iDataLen：流数据长度
// iWidth：图像宽度
// iHeight：图像高度
// iFrameType：图像YUV类型（目前为YV12，值为3）
// iTimeStamp：时间戳
// pUserData：用户数据
typedef void(CALLBACK* pfnDecodedDataCallback)(VIDEO_INT64 i64PlayHandle, const char* pDataArray, int iDataLen, int iWidth, int iHeight, int iFrameType, int iTimeStamp, void* pUserData);

// 录像下载回调（进入和消息）
// i64DownloadHandle：网络录像下载句柄
// fPercent：已下载录像进度，当iMsg为0时进度有效
// iMsg：录像下载消息，0-开始录像下载 1-录像下载中 2-录像下载完成 3-录像下载即将分包 4-录像下载分包失败 5-录像下载分包完成 6-录像下载时断流 30-转封装不支持 50-断点续传不支持 100-下载失败（内部错误）
// pUserData：用户自定义数据
typedef void (CALLBACK* pfnDownloadCallback)(VIDEO_INT64 i64DownloadHandle, float fPercent, int iMsg, void* pUserData);

// 直连萤石录像回放查询结果回调
// i64PlayHandle：回放接口返回的句柄
// iErrorCode：查询录像错误码，查询失败时，返回的错误码信息
// pszRecordSegments：录像片段字符串
typedef long long (CALLBACK* pfnRecordSegmentCallback)(VIDEO_INT64 i64PlayHandle, int iErrorCode, const char* pszJsonRecordSegments, int iDataLen, void* pUserData);

// 直连萤石更新token回调
// pUserData: 用户自定义数据
typedef long long (CALLBACK* pfnUpdataEzvizTokenCallback)(void* pUserData);

////////////////////////// 结构体定义 //////////////////////////////////
typedef struct stPlayReq
{
    int iHardWareDecode;                // 是否开启GPU硬解 0-不开启 1-开启（如果开启硬解，但如显卡不支持等导致错误，SDK内部自动切换成软解；开启硬解只是增加显示性能，并不能减少内存占用率，如果不是特别需求，建议不开启硬解）
    const char* strUsrmsg;              //hps会话参数，设置水印
	const char* appkey;					//合作方的ak,sk
	const char* secretkey;			
    pfnStreamCallback fnStream;         // 码流数据回调
    pfnMsgCallback fnMsg;               // 播放、取流消息回调
    pfnDecodedDataCallback fnDecodedStream;     // 解码后的YUV数据回调，注意开启了GPU硬解不支持回调解码后数据，即时指定了回调也不起效
    void* pUserData;                    // 用户数据
    char szReserve[64];                 // 填充“smallEagleEye”标识小鹰眼设备（碗状鹰眼设备），不填充为普通设备，填充其它值-未定义（如需要填充，请勿填充除“smallEagleEye”外的值）
}VIDEO_PLAY_REQ, *PVIDEO_PLAY_REQ;

typedef struct stFilePlayReq
{
    pfnMsgCallback fnMsg;               // 播放、取流消息回调
    void* pUserData;                    // 用户数据
    char szReserve[64];                 // 保留参数
}VIDEO_FILEPLAY_REQ, *PVIDEO_FILEPLAY_REQ;

typedef struct stEzvizRealPlayReq
{
    int iStreamType;                    // 主子码流类型，1-主码流 0-子码流
    int iChannelNo;                     // 萤石设备通道号
	int iHardWareDecode;                // 是否开启GPU硬解 0-不开启 1-开启（如果开启硬解，但如显卡不支持等导致错误，SDK内部自动切换成软解；开启硬解只是增加显示性能，并不能减少内存占用率，如果不是特别需求，建议不开启硬解）
    pfnStreamCallback fnStream;         // 码流数据回调
    pfnMsgCallback fnMsg;               // 播放、取流消息回调
    pfnDecodedDataCallback fnDecodedStream;         // 解码后的YUV数据回调
    void* pUserData;                    // 用户数据
    char szReserve[64];                 // 保留参数
}VIDEO_EZVIZ_REALPLAY_REQ, *PVIDEO_EZVIZ_REALPLAY_REQ;

typedef struct stEzvizRecordReq
{
    int iChannelNo;                         // 萤石设备通道号
	int iHardWareDecode;                    // 是否开启GPU硬解 0-不开启 1-开启（如果开启硬解，但如显卡不支持等导致错误，SDK内部自动切换成软解；开启硬解只是增加显示性能，并不能减少内存占用率，如果不是特别需求，建议不开启硬解）
    pfnStreamCallback fnStream;             // 码流数据回调
    pfnMsgCallback fnMsg;                   // 播放、取流消息回调
    pfnDecodedDataCallback fnDecodedStream; // 解码后的YUV数据回调
    pfnRecordSegmentCallback fnSegment;     // 直连萤石回放录像片段回调
    void* pUserData;                        // 用户数据
    VIDEO_INT64 i64StartTimeStamp;          // 查询录像开始时间戳，单位：秒
	VIDEO_INT64 i64PlayTimeStamp;           // 录像播放开始时间戳，单位：秒
    VIDEO_INT64 i64EndTimeStamp;            // 查询录像结束时间戳，单位：秒
    char szReserve[64];                     // 保留参数
}VIDEO_EZVIZ_RECORDPLAY_REQ, *PVIDEO_EZVIZ_RECORDPLAY_REQ;

typedef struct stDownloadReq
{
	const char* appkey;					//合作方的ak,sk
	const char* secretkey;
    pfnDownloadCallback fnDownload;     // 下载进度回调
    void* pUserData;                    // 用户数据
    VIDEO_INT64 i64FileMaxSize;         // 录像分包大小
    VIDEO_INT64 i64RecordSize;          // 录像总大小，请填充查询回放URL时查询到的各录像片段大小之和
    VIDEO_INT64 i64StartTimeStamp;      // 录像下载开始时间，请填充查询回放URL时的查询开始时间戳，单位：秒
    VIDEO_INT64 i64EndTimeStamp;        // 录像下载结束时间，请填充查询回放URL时的查询结束时间戳，单位：秒
    char szReserve[64];             	// 录像类型，填充“0”或不填充-普通录像下载，“1”-录像转封装，如果音频或视频不支持则回调不支持消息 “2” - 录像转封装，但音频或视频不支持时按不转封装处理 “3”-录像转封装，如果音频不支持则只转视频，如果视频也不支持则回调不支持消息 “4”-启用断点续传 其它值 - 参数错误（对于指定录像转封装和启用断点续传，目前仅支持海康SDK协议、EHOME协议接入设备，如果是其它协议接入的设备，会回调不支持的消息；对于断点续传，只支持未转封装的录像；调录像下载时可以先查询设备接入协议，非海康SDK、EHOME协议不填充或填充“0”即可；转封装是将视频转换成标准MP4文件，可以在通用播放器上播放，如不转封装，需要专用播放器播放，如大华SDK接入设备下载的录像需要大华播放器，海康SDK协议/EHOME协议下载录像需要海康播放器播放）
}VIDEO_DOWNLOAD_REQ, *PVIDEO_DOWNLOAD_REQ;

typedef struct stTalkReq
{
	const char* appkey;					//合作方的ak,sk
	const char* secretkey;
	pfnMsgCallback fnMsg;				// 语音对讲消息，用于回调语音对讲取流消息，如不需要可传NULL。
	void* pUserData;					// 用户数据，用于fnMsg回调中透传出去，如不需要可传NULL。
	char szReserve[64];					// 预留参数
}VIDEO_TALK_REQ,*PVIDEO_TALK_REQ;


typedef struct stVideoDetailInfo
{
    int iWidth;                         // 视频图像宽度
    int iHeight;                        // 视频图像高度
    int iEncodeType;                    // 编码类型  1-H264 2-MPEG2_PS 3-MPEG4 4-H265 5-GB28181 6-raw 7-vag 8-dahua 9-smartH264 10-smartH265
    int iEncapsulationType;             // 封装类型  1-ps 2-ts 3-rtp 4-raw 5-rtp + ps
    VIDEO_INT64 i64FrameRate;           // 帧率    
    char szReserve[64];                 // 保留参数
	float fCodeRate;                    // 码率（V1.2.0新增）
}VIDEO_DETAIL_INFO, *PVIDEO_DETAIL_INFO;

typedef struct stOSDInfo
{
    int iBold;                  // 叠加字符串是否粗体 1-粗体 0-非粗体
    int ix;                     // 叠加字符相对与播放窗口左上角起点的横坐标
    int iy;                     // 叠加字符相对与播放窗口左上角起点的纵坐标
    int iFontSize;              // 字体大小
    int iAlignType;             // 对齐方式，0-左对齐 1-居中对齐 2-右对齐
    VIDEO_INT64 i64Color;       // 字符颜色，Windows中使用RGB宏获得颜色值
    char szReserve[64];         // 保留参数
}VIDEO_OSD_INFO, *PVIDEO_OSD_INFO;

typedef struct stRectangleInfo
{
    int iLeftX;                 // 左上顶点横坐标
    int iLeftY;                 // 左上顶点纵坐标
    int iRightX;                // 右下顶点横坐标
    int iRightY;                // 右下顶点纵坐标
    VIDEO_INT64 i64Color;     // 线条颜色，Windows中使用RGB宏获得颜色值
    int iLineWidth;                // 线条粗细
    char szReserve[64];          // 保留参数
}VIDEO_RECTANGLE_INFO, *PVIDEO_RECTANGLE_INFO;

typedef struct stDisplayInfo
{
    float fx;                 // 鼠标在播放窗口上拖动形成矩形的起点的水平位置（相对播放窗口左上角）
    float fy;                 // 鼠标在播放窗口上拖动形成矩形的起点的垂直位置（相对播放窗口左上角）
    float fWidth;             // 鼠标在播放窗口上拖动形成矩形的宽度
    float fHeight;            // 鼠标在播放窗口上拖动形成矩形的高度
    int iOperType;           // 0-缩小 1-放大
	bool bMove;              //true-平移电子放大，false-连续电子放大
    char szReserve[64];      // 保留参数
}VIDEO_DISPLAY_INFO, *PVIDEO_DISPLAY_INFO;

typedef struct stSmallEagleRotateInfo
{
    float fx;                 // x坐标
    float fy;                 // y坐标
    float fz;                 // z坐标
    float fv;                 // 放大缩小值，大于0.0则放大，否则缩小
    char szReserve[64];      // 保留参数
}VIDEO_SMALLEAGLEROTATE_INFO, *PVIDEO_SMALLEAGLEROTATE_INFO;

////////////////////////// 接口定义 /////////////////////////////
// SDK初始化
// pszEvn：保留参数，传NULL即可
// 备注：调一次即可，初始化成功情况下重复调直接返回成功
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_Init(_IN_ const char* pszEvn);

// SDK反初始化
// 备注：调一次即可，反初始化成功情况下重复调直接返回成功
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_Fini();

// 同步开始视频预览（支持取实时流、回调取流与播放的消息，以及回调YUV数据，需要播放请求参数中指定）
// pszUrl：预览URL，每次预览需要重新查询URL
// hWnd：视频预览的Windows窗口句柄
// pstPlayReq：播放请求参数，具体参数详见结构体定义
// 同步返回结果，成功返回大于0的预览句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartPreview(_IN_ const char* pszUrl, _IN_ void* hWnd, _IN_ PVIDEO_PLAY_REQ pstPlayReq);

// 异步开始视频预览（支持取实时流、回调取流与播放的消息，以及回调YUV数据，需要播放请求参数中指定）
// pszUrl：预览URL，每次预览需要重新查询URL
// hWnd：视频预览的Windows窗口句柄
// pstPlayReq：播放请求参数，具体参数详见结构体定义
// 异步返回结果，成功返回大于0的预览句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码；返回值只表示发起异步请求的结果，不代表发起取流请求的结果，取流请求结果成功与否参考消息回调中的“异步发起取流请求成功”与“异步发起取流请求失败”消息
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartAsynPreview(_IN_ const char* pszUrl, _IN_ void* hWnd, _IN_ PVIDEO_PLAY_REQ pstPlayReq);

// 停止预览（停止取流，停止回调消息，停止回调YUV数据、停止直连预览）
// i64PlayHandle：预览句柄，来源于Video_StartPreview、Video_StartEzvizPreview返回值
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StopPreview(_IN_ VIDEO_INT64 i64PlayHandle);

// 即时回放
// i64PlayHandle:预览句柄，来源于Video_StartPreview返回值
// iCtrlType:即时回放控制参数，0-进入即时回放 1-退出即时回放，其它值-失败
// 备注：预览即时回放是指在使用预览的窗口播放预览过程中缓存的码流。码流是否缓存，缓存多少可由VideoSDK.ini配置文件（配置项详见配置文件清单）决定，默认开启缓存，缓存5M码流。如果配置文件配置不缓存，则调此接口进入即时回放会导致失败，错误码为不支持。
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_InstantPlay(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iCtrlType);

// 保存即时回放
// i64PlayHandle:预览句柄，来源于Video_StartPreview返回值
// pszFileName:录像文件绝对路径文件名称，后缀必须是“mp4”，如“E:\\test.mp4”，路径和文件名称中不能含有特殊字符（路径中目录名称全是空格字符串，中英文下的“*”、“ | ”以及英文下的“ ? ”等特殊字符串）
// recordType：录像类型，0-不转封装，1-录像转封装如果音频或视频不支持则返回错误 2-录像转封装但音频或视频不支持时按不转封装处理 3-录像转封装如果音频不支持则只转视频 其它值-参数错误（1~3针对海康流，如果当前视频是非海康流，接口返回不支持）指定0时该接口为同步接口，指定1~3为异步接口，即保存结果通过消息回调给出，定义详见预览消息回调
// 备注：进入即时回放后调此接口保存即时回放，如果未进入即时回放调此接口将导致失败，错误码为操作次序不对
// 如果指定了转封装并且没有转封装完成，只有当退出时即时回放时才会结束转封装
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_SaveInstantPlay(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ const char* pszAbsoluteFileName, _IN_ int recordType);

// 查询即时回放总时长
// i64PlayHandle：预览句柄，来源于Video_StartPreview的返回值
// 备注：进入即时回放后调此接口获取即时回放总时长，如果未进入即时回放调此接口将导致失败，错误码为操作次序不对
// 成功返回大于等于0的总时长，失败返回VIDEO_ERR_FAIL，单位：秒。接口返回失败通过调用Video_GetLastError获取错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_GetInstantPlayDuration(_IN_ VIDEO_INT64 i64PlayHandle);

// 查询即时回放已播放时间
// i64PlayHandle：预览句柄，来源于Video_StartPreview的返回值
// 备注：进入即时回放后调此接口获取即时回放总时长，如果未进入即时回放调此接口将导致失败，错误码为操作次序不对
// 成功返回大于等于0的已播放时长，失败返回VIDEO_ERR_FAIL，单位：秒。接口返回失败通过调用Video_GetLastError获取错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_GetInstantPlayedTime(_IN_ VIDEO_INT64 i64PlayHandle);

// 同步网络录像回放、倒放（支持取网络录像流、回调取流与播放的消息，以及回调YUV数据，需要播放请求参数中指定）
// pszUrl：回放URL
// hWnd：网络录像正放或倒放的Window窗口句柄
// i64StartTimeStamp：正放时为查询回放URL时的开始时间戳，倒放时为查询回放URL时的结束时间戳，单位：秒
// i64EndTimeStamp：正放时为查询回放URL时的结束时间戳，倒放时为查询回放URL时的开始时间戳，单位：秒
// pstPlayReq：播放请求参数，详细参数详见结构体定义
// 同步返回结果，成功返回大于0的播放句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartPlayback(_IN_ const char* pszUrl, _IN_ void* hWnd, _IN_ VIDEO_INT64 i64StartTimeStamp, _IN_ VIDEO_INT64 i64EndTimeStamp, _IN_ PVIDEO_PLAY_REQ pstPlayReq);

// 异步网络录像回放、倒放（支持取网络录像流、回调取流与播放的消息，以及回调YUV数据，需要播放请求参数中指定）
// pszUrl：回放URL
// hWnd：网络录像正放或倒放的Window窗口句柄
// i64StartTimeStamp：正放时为查询回放URL时的开始时间戳，倒放时为查询回放URL时的结束时间戳，单位：秒
// i64EndTimeStamp：正放时为查询回放URL时的结束时间戳，倒放时为查询回放URL时的开始时间戳，单位：秒
// pstPlayReq：播放请求参数，详细参数详见结构体定义
// 异步返回结果，成功返回大于0的播放句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码；返回值只表示发起异步请求的结果，不代表发起取流请求的结果，取流请求结果成功与否参考消息回调中的“异步发起取流请求成功”与“异步发起取流请求失败”消息
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartAsynPlayback(_IN_ const char* pszUrl, _IN_ void* hWnd, _IN_ VIDEO_INT64 i64StartTimeStamp, _IN_ VIDEO_INT64 i64EndTimeStamp, _IN_ PVIDEO_PLAY_REQ pstPlayReq);

// 停止网络录像正放、倒放（停止取流，停止回调消息，停止回调YUV数据）
// i64PlayHandle：网络录像播放句柄，来源于Video_StartPlayback的返回值
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StopPlayback(_IN_ VIDEO_INT64 i64PlayHandle);

// 查询录像回放当前播放时间戳
// i64PlayHandle:网络录像播放句柄，来源于Video_StartPlayback、Video_StartEzvizPlayback的返回值
// 成功返回大于等于0的当前回放时间戳，失败返回VIDEO_ERR_FAIL，单位：秒。接口返回失败通过调用Video_GetLastError获取错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_GetCurrentPlayTime(_IN_ VIDEO_INT64 i64PlayHandle);

//------------------直连萤石-------------------
// 初始化萤石直连
// pszEnv：从萤石平台申请的APPKey
// pszEzvizToken：从萤石平台获取的token
// fnEzvizToken：更新萤石token回调
// pUserData：用户数据
// 备注：需要在SDK初始化后再初始化直连萤石，调一次即可，初始化成功情况下如果使用相同的萤石APPKey和token初始化接口返回成功，这种情况不会更新回调，如果使用不同的APPKey和token将返回失败；如果使用方自己定时更新萤石token，fnEzvizToken与pUserData可传入NULL，此时将不再通知萤石token即将过期消息
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_InitEzviz(_IN_ const char* pszEzvizAppkey, _IN_ const char* pszEzvizToken, _IN_ pfnUpdataEzvizTokenCallback fnEzvizToken, _IN_ void* pUserData);

// 萤石反初始化
VIDEOSDK_DECLARE void Video_UninitEzviz();

// 更新萤石token，当萤石token即将失效时，通过pfnUpdataEzvizTokenCallback回调更新萤石token消息或使用方自行定时更新萤石token，使用方查询萤石token后通过此接口更新萤石token
// pszEzvizToken：从萤石平台获取的token
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_UpdateEzvizToken(_IN_ const char* pszEzvizToken);

// 直连萤石预览、取流
// hWnd：预览窗口句柄
// pszDevSerial：萤石设备序列号
// pszSafeKey：视频加密秘钥，如未加密请填充NULL。
// pstPlayReq：直连萤石预览请求参数，详细参数见结构体定义
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartEzvizPreview(_IN_ void* hWnd, _IN_ const char* pszDevSerial, _IN_ const char* pszSafeKey, _IN_ PVIDEO_EZVIZ_REALPLAY_REQ pstEzvizRealPlayReq);

// 直连萤石回放、取流
// hWnd：回放窗口句柄
// pszDevSerial:萤石设备序列号
// pszSafeKey：视频加密秘钥，如未加密请填充NULL.
// pszPlayReq：直连萤石回放请求参数
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartEzvizPlayback(_IN_ void* hWnd, _IN_ const char* pszDevSerial, _IN_ const char* pszSafeKey, _IN_ PVIDEO_EZVIZ_RECORDPLAY_REQ pstEzvizRecordReq);

// 直连萤石语音对讲
// pszDevSerial:萤石设备序列号
// iChannelNo:萤石设备通道号
// pszFileName:语音对讲保存音频的绝对路径文件名称，只支持wav后缀，如“D:\test.wav”。如不需要录音，该参数请传NULL。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”。暂时不支持保存音频文件功能，该参数请填充NULL。
// fnMsg:语音对讲消息回调函数，，用于回调语音对讲取流消息，如不需要可传NULL
// pUserData:用户数据，用于fnMsg回调中透传出去，如不需要可传NULL
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StartEzvizTalk(_IN_ const char* pszDevSerial, _IN_ int iChannelNo, _IN_ const char* pszFileName, _IN_ pfnMsgCallback fnMsg, _IN_ void* pUserData);

//-----------文件播放--------------
// 开始文件播放
// pszFileName：绝对路径mp4文件名称
// hWnd：文件播放窗口句柄
// pstFilePlayReq：文件播放请求参数结构体，不需回调消息，可传NULL
// 备注：仅支持播放通过海康SDK协议、Ehome/ISUP协议、ONVIF协议、大华SDK协议、国标协议、萤石接入的设备紧急录像、录像剪辑和录像下载的文件
// 成功返回大于0的文件播放句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartFilePlay(_IN_ const char* pszFileName, _IN_ void* hWnd, _IN_ PVIDEO_FILEPLAY_REQ pstFilePlayReq);

// 停止文件播放
// i64PlayHandle：文件播放句柄，来源于Video_StartFilePlay的返回值
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StopFilePlay(_IN_ VIDEO_INT64 i64PlayHandle);

// 查询文件播放总时长
// i64PlayHandle:文件播放句柄，来源于Video_StartFilePlay的返回值
// 成功返回大于等于0的总时长，失败返回VIDEO_ERR_FAIL，单位：秒。接口返回失败通过调用Video_GetLastError获取错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_GetFilePlayDuration(_IN_ VIDEO_INT64 i64PlayHandle);

// 查询文件已播放时长
// i64PlayHandle：文件播放句柄，来源于Video_StartFilePlay的返回值
// 成功返回大于等于0的已播放时长，失败返回VIDEO_ERR_FAIL，单位：秒。接口返回失败通过调用Video_GetLastError获取错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_GetFilePlayedTime(_IN_ VIDEO_INT64 i64PlayHandle);

//-----------------录像下载-----------------
// 开始录像下载
// pszUrl:正放取流URL
// pszFileName:录像文件绝对录像名称，请指定后缀mp4，如“E:\test.mp4”。pszFileName为ansi编码字符串。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”。录像下载不支持下载到网络位置,只能下载到本地。对于pstDownloadReq中指定断点续传时，请传入上次录像下载最后生成的一个文件的绝对路径文件名称，如果不支持会通过回调给出消息
// pstDownloadReq:录像下载请求参数，详细参数详见结构体定义
// 备注：支持通过配置文件VideoSDK.int配置：非海康流时，取流缓冲大小；海康流时是否转封装已支持普通播放器播放
// 成功返回大于0的播放句柄，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE VIDEO_INT64 Video_StartDownload(_IN_ const char* pszUrl, _IN_ const char* pszFileName, _IN_ PVIDEO_DOWNLOAD_REQ pstDownloadReq);

// 停止下载
// i64DownloadHandle：录像下载句柄，来源于Video_StartDownload的返回值
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StopDownload(_IN_ VIDEO_INT64 i64DownloadHandle);

//-----------------通用接口--------------------
// 开始本地录像，对于同一路，同一时刻只支持一路本地录像，否则返回不支持
// i64PlayHandle：播放句柄，Video_StartPlayback、Video_StartPreview、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// pszFileName:录像文件绝对路径名称，请指定后缀mp4，如“E:\test.mp4”。pszFileName为ansi编码字符串。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”。
// i64FileMaxSize：分包大小单位：字节，不得小于5M，否则按5M计算；分包后命名规则：“E:\test-1.mp4”、...“E:\test-n.mp4”，n表示除“E:\test.mp4”外的第n个分包
// recordType：录像类型，0-不转封装，1-录像转封装如果音频或视频不支持则返回错误 2-录像转封装但音频或视频不支持时按不转封装处理（前提是存在音频、视频数据） 3-录像转封装如果音频不支持则只转视频 其它值-参数错误（1~3针对海康流，如果当前视频是非海康流，接口返回不支持）
// 有些设备出的混合流中音频流不支持转封装，此时只将视频流转封装
// 有些设备出的流不支持转封装（如大华设备），此时将不转封装
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StartLocalRecord(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ const char* pszFileName, _IN_ VIDEO_INT64 i64FileMaxSize, _IN_ int recordType);

// 停止本地录像
// 成功返回码流存文件个数，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_StopLocalRecord(_IN_ VIDEO_INT64 i64PlayHandle);

//播放、下载控制
// i64PlayHandle:播放句柄，Video_StartPlayback、Video_StartPreview、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iCtrlType:0-暂停 1-恢复播放 2-定位播放 3-倍速播放/下载 4-暂停下载 5-恢复下载 6-单帧控制
// i64Param:控制参数，与iCtrlType配合使用
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：1-恢复播放是恢复到1倍速，而不是上次的播放速度；2-时间戳参数由调用方保证介于开始与结束时间戳之间，单位：秒；3-控制参数为速度取值：16、8、4、2、1、-2、-4、-8、-16，依次由快放至慢放；6-控制参数取值：0-单帧进 1-单帧退 2-退出单帧
VIDEOSDK_DECLARE int Video_PlayCtrl(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iCtrlType, _IN_ VIDEO_INT64 i64Param);

//主子码流切换
// iPlayHandle:播放句柄，Video_StartPreview、Video_StartEzvizPreview返回
// iAsyn：是否异步切换，1-异步，0-同步；如果是异步切换，切换成功消息由消息回调给出，否则接口返回值表明是否切换成功，取其它值则返回失败
// pszParam：主子码流切换参数，非直连萤石时指定码流类型的预览或取流url；直连萤石预览切换主子码流时，请置为NULL
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：直连萤石预览或取流时，默认切换至不同于当前码流类型的另一种码流类型。如直连萤石主码流预览时，调用此接口将默认切换为子码流；直连萤石子码流预览时，调用此接口将默认切换为主码流。
VIDEOSDK_DECLARE int Video_ChangeStreamType(VIDEO_INT64 i64PlayHandle, int iAsyn, const char* pszParam);

// 播放抓图
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// pszFileName:要保存的图片绝对路径（含文件名），根据文件名后缀确定保存的图片格式（支持bmp、jpg），例：“E:/SnapShot/测试监控点.jpg”。pszFileName为ansi编码字符串。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_PlaySnap(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ const char* pszFileName);

// 声音控制
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iCtrlType:控制类型，0-打开 1-关闭，其它值-未定义。
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：声音功能需要流数据中有音频数据才有效。
VIDEOSDK_DECLARE int Video_SoundCtrl(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iCtrlType);

// 获取音量
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// 成功返回音量值，音量范围[0,100]，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：需要打开声音后才有效。
VIDEOSDK_DECLARE int Video_GetVolume(_IN_ VIDEO_INT64 i64PlayHandle);

// 设置音量
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iVolume:音量值，范围[0,100]
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：需要打开声音后才有效。
VIDEOSDK_DECLARE int Video_SetVolume(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iVolume);

// 电子放大
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iCtrlType:标志进入电子放大还是退出电子放大，0-进入电子放大 1-退出电子放大 其它值-参数错误。
// hWnd:进入电子放大时，全景窗口句柄。
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注:小鹰眼不支持电子放大。
VIDEOSDK_DECLARE int Video_DigitalZoom(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iCtrlType, _IN_ void* hWnd);

// 设置电子放大显示区域
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// pstDisplayInfo:电子放大显示区域参数
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注:进入电子放大后设置电子放大显示区域。
VIDEOSDK_DECLARE int Video_SetDisplayZoom(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ PVIDEO_DISPLAY_INFO pstDisplayInfo);

// 画面字符叠加
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iOSDId:字符叠加标识，第一次叠加时请传入0，更新画面上的字符串时填入第一次叠加时返回的Id
// pszText:待叠加字符串，支持“\n”换行，字符串不超过512个字节，如超过则截断显示
// pstOSDInfo:字符叠加信息
// 成功返回本次叠加的id，该Id取值大于等于0，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_SetOSDText(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iOSDId, _IN_ const char* pszText, _IN_ PVIDEO_OSD_INFO pstOSDInfo);

// 画面矩形框叠加
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iDrawType:绘制类型，0-实时预览、录像回放、即时回放和文件播放指定的窗口上绘制 1-电子放大指定的窗口上绘制
// pstRectangleInfo:矩形叠加信息
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注:电子放大时使用该接口在画面上绘制矩形框，以标记处放大区域
VIDEOSDK_DECLARE int Video_DrawRectangle(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iDrawType, _IN_ PVIDEO_RECTANGLE_INFO pstRectangleInfo);


// 画面自定义图形框叠加
// i64PlayHandle:播放句柄，Video_StartPreview、Video_StartPlayback、Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback接口返回
// iGraphType:绘制类型，0-矩形 1-直线
// pstRectangleInfo:矩形叠加信息
// lineMode 批量划线，如果是因为窗口大小变化的重绘，传1，新增加的，传0
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_CustomDraw(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iGraphType, _IN_ PVIDEO_RECTANGLE_INFO pstRectangleInfo, _IN_  int lineMode);
// 开始语音对讲
// pszUrl:语音对讲URL，可从OpenAPI查询对讲URL。每次对讲都需重新查询对讲URL
// pszFileName:语音对讲保存音频的绝对路径文件名称，只支持wav后缀，如“D:\test.wav”。如不需要录音，该参数请传NULL。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”。暂时不支持保存音频文件功能，该参数请填充NULL。
// pstTalkReq:语音对讲结构体
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：同一时刻只支持一路对讲，暂无法区分语音对讲失败还是语音对讲通道被占用。只支持与前端设备对讲，前端IPC设备接入NVR后，支持与前端IPC设备对讲，不支持直接与NVR对讲。每次对讲都需重新查询对讲URL。暂不支持保存音频文件功能，pszFileName参数请填充NULL。语音对讲只支持海康SDK和Ehome协议接入设备。
VIDEOSDK_DECLARE int Video_StartTalk(_IN_ const char* pszUrl, _IN_ const char* pszFileName, PVIDEO_TALK_REQ pstTalkReq);

// 停止语音对讲
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL。接口返回失败请调用Video_GetLastError获取错误码，通过错误码判断出错原因
VIDEOSDK_DECLARE int Video_StopTalk();

// 查询视频详细信息
// i64PlayHandle：实时预览、网络录像正放或倒放句柄，来源于Video_StartPreview或Video_StartPlayback、Video_StartFilePlay返回值
// pstVideoDetailInfo：视频详细信息结构体指针
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL。接口返回失败请调用Video_GetLastError获取错误码，通过错误码判断出错原因
VIDEOSDK_DECLARE int Video_GetVideoInfo(_IN_ VIDEO_INT64 i64PlayHandle, _OUT_ PVIDEO_DETAIL_INFO pstVideoDetailInfo);

// 私有数据控制（指画面上的如移动侦测的线框等数据的开关控制，针对海康的设备，支持海康SDK、EHOME/ISUP、ONVIF、GB28181协议接入）
// i64PlayHandle：句柄参数，来源于Video_StartPreview、Video_StartPlayback或Video_StartFilePlay的返回值
// iPrivateDataType：私有数据类型，多个可使用“|”运算进行批量控制，0x0001-智能分析 0x0002-移动侦测 0x0004-POS信息后叠加 0x0008-图片叠加 0x0010-热成像信息 0x0020-温度信息
// iCtrlType：显示隐藏开关 0-显示 1-隐藏
// iParam：一些私有数据类型存在子类型，当iDataType为0x0010时，子类型有0x0001-点火框显示 0x0002-最高温度 0x0004-最高温度位置 0x0008-最高温度距离；当iDataType为0x0020时，子类型有0x0001-框测温 0x0002-线测温 0x0004-点测温
// 注：对于不涉及子类型（或不需子类型）的，支持各个私有数据类型的“|”运算，对于涉及子类型的，需一个一个对主类型进行控制，同一类型的子类型支持“|”运算；iDataType为0x0010和0x0020时需要热成像设备支持，其它类型普通设备即可支持，但需要设备上配置
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码，非海康设备不支持会，目前无法区分是否支持
VIDEOSDK_DECLARE int Video_PrivateDataCtrl(_IN_ VIDEO_INT64 iPlayHandle, _IN_ int iPrivateDataType, _IN_ int iCtrlType, _IN_ int iParam);

// 小鹰眼播放控制
// i64PlayHandle：句柄参数，来源于Video_StartPreview、Video_StartPlayback或Video_StartFilePlay的返回值
// iMode：小鹰眼画面显示模式，0-八画面显示 1-碗状画面显示
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
// 备注：仅支持海康SDK协议接入的设备，如以其它设备接入协议接入则当做普通设备处理。如果不指定小鹰眼画面显示模式，则默认显示4画面。小鹰眼设备实时预览不支持即时回放，网络录像回放不支持倒放、不支持单帧退
VIDEOSDK_DECLARE int Video_SetSmallEagleEyeMode(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ int iMode);

// 小鹰眼碗状画面时旋转控制
// i64PlayHandle：句柄参数，来源于Video_StartPreview、Video_StartPlayback或Video_StartFilePlay的返回值
// pstRotateInfo：小鹰眼碗状画面旋转参数
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_SetSmallEagleEyeRotate(_IN_ VIDEO_INT64 i64PlayHandle, _IN_ PVIDEO_SMALLEAGLEROTATE_INFO pstRotateInfo);

// 刷新播放
// i64PlayHandle：句柄参数，来源于Video_StartPreview 、Video_StartPlayback或Video_StartFilePlay、Video_StartEzvizPreview或Video_StartEzvizPlayback返回值
// 成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码
VIDEOSDK_DECLARE int Video_RefreshPlay(VIDEO_INT64 i64PlayHandle);

// 获取错误码
VIDEOSDK_DECLARE int Video_GetLastError();

// 异步开始语音对讲
// pszUrl:语音对讲URL，可从OpenAPI查询对讲URL。每次对讲都需重新查询对讲URL
// pszFileName:语音对讲保存音频的绝对路径文件名称，只支持wav后缀，如“D:\test.wav”。如不需要录音，该参数请传NULL。路径和文件名称不能包含特殊字符（中英文下的“*”、“|”以及英文下的“?”），此外，路径中慎用“.”和“..”。暂时不支持保存音频文件功能，该参数请填充NULL。
// fnMsg:语音对讲消息，用于回调语音对讲取流消息，如不需要可传NULL。
// pUserData:用户数据，用于fnMsg回调中透传出去，如不需要可传NULL。
// 异步返回结果，成功返回VIDEO_ERR_SUCCESS，失败返回VIDEO_ERR_FAIL，失败了通过Video_GetLastError查询错误码；返回值只表示发起异步请求的结果，不代表发起取流请求的结果，取流请求结果成功与否参考消息回调中的“异步发起取流请求成功”与“异步发起取流请求失败”消息
// 备注：同一时刻只支持一路对讲，暂无法区分语音对讲失败还是语音对讲通道被占用。只支持与前端设备对讲，前端IPC设备接入NVR后，支持与前端IPC设备对讲，不支持直接与NVR对讲。每次对讲都需重新查询对讲URL。暂不支持保存音频文件功能，pszFileName参数请填充NULL。语音对讲只支持海康SDK和Ehome协议接入设备。
VIDEOSDK_DECLARE int Video_StartAsynTalk(_IN_ const char* pszUrl, _IN_ const char* pszFileName, PVIDEO_TALK_REQ pstTalkReq);

#endif  // HIK_VIDEOSDK_H