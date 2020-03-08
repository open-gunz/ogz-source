/*
 *	CMErrorDef.h
 *		Error처리를 위한 코드
 *		이장호 ( 98-01-04 3:31:28 오전 )
 *
 *		에러 코드명 앞에 ESTR_을 붙이면, 영문 버전에서 출력되는
 *			에러 메세지이고,
 *		KSTR_을 붙이면, 한글 버전에서 출력되는 에러 메세지이다.
 ********************************************************************/
#ifndef _CMERRORDEF_H
#define _CMERRORDEF_H

/*	OK	*/
#define CM_OK						0
#define ESTR_CM_OK					"OK."
#define KSTR_CM_OK					"정상입니다."

/*	Generic Error	*/
#define CMERR_GENERIC				100
#define ESTR_CMERR_GENERIC			"Generic Error Occured."
#define KSTR_CMERR_GENERIC			"일반적인 에러입니다."

/*	Out of Memory	*/
#define CMERR_OUT_OF_MEMORY			101
#define ESTR_CMERR_OUT_OF_MEMORY	"Out of Memory."
#define KSTR_CMERR_OUT_OF_MEMORY	"메모리가 부족합니다."

/*	pErrStr : 오류가 발생한 파일 이름	*/
#define CMERR_CANT_OPEN_FILE		102
#define ESTR_CMERR_CANT_OPEN_FILE	"Can't Open File."
#define KSTR_CMERR_CANT_OPEN_FILE	"파일을 열 수 없습니다."

/*	pErrStr : 오류가 발생한 파일 이름	*/
#define CMERR_CANT_SAVE_FILE		103
#define ESTR_CMERR_CANT_SAVE_FILE	"Can't Save File."
#define KSTR_CMERR_CANT_SAVE_FILE	"파일을 저장할 수 없습니다."

/*	Cannot Create Window Error	*/
/*	Modified by CJP */
#define CMERR_CANT_CREATE_WIN		104
#define ESTR_CMERR_CANT_CREATE_WIN	"Can't not create a window."
#define KSTR_CMERR_CANT_CREATE_WIN	"윈도우를 생성할 수 없습니다."

/* 파일이 깨지거나 레코드의 내용을 참조할 수 없는 경우 */
#define CMERR_INVALID_FILE			105
#define ESTR_CMERR_INVALID_FILE		"The file is invalid. Please re-install or find a patch program."
#define KSTR_CMERR_INVALID_FILE		"파일이 유효하지 않습니다. 다시 설치하거나 패치 프로그램을 찾아 설치하시기 바랍니다."

/*	Error Code by Joongpil, Cho	*/
#define CMERR_JOONGPIL_CHO					1000

#ifdef ACESAGA	// AceSaga Specific

#define	CMERR_CANNOT_OPEN_UITEX				1001
#define ESTR_CMERR_CANNOT_OPEN_UITEX		"Cannot open ui.txz."
#define KSTR_CMERR_CANNOT_OPEN_UITEX		"Data\\ui.txz를 읽어올 수 없습니다."

#define	CMERR_FAIL_INIT_RS					1002
#define ESTR_CMERR_FAIL_INIT_RS				"Cannot init RealSpace. Please configure for your system."
#define KSTR_CMERR_FAIL_INIT_RS				"RealSpace를 초기화할 수 없습니다. Config 유틸리티를 이용하여 다시 설정해주세요."

#endif

#ifdef ARDNET	// ArdNet Specific

#define	CMERR_CANNOT_OPEN_UITEX				1001
#define ESTR_CMERR_CANNOT_OPEN_UITEX		"Cannot open ui.txz."
#define KSTR_CMERR_CANNOT_OPEN_UITEX		"Data\\ui.txz를 읽어올 수 없습니다."

#define	CMERR_FAIL_INIT_RS					1002
#define ESTR_CMERR_FAIL_INIT_RS				"Cannot init RealSpace. Please configure for your system."
#define KSTR_CMERR_FAIL_INIT_RS				"RealSpace를 초기화할 수 없습니다. Config 유틸리티를 이용하여 다시 설정해주세요."

#define CMERR_FAIL_OPEN_MAPFILE				1003
#define ESTR_CMERR_FAIL_OPEN_MAPFILE		"Cannot open the map file. Please re-install Ard|Net."
#define KSTR_CMERR_FAIL_OPEN_MAPFILE		"지정한 맵파일을 읽어올 수 없습니다. 다시 설치해주시기 바랍니다."

#define CMERR_FAIL_OPEN_ATTFILE				1004
#define ESTR_CMERR_FAIL_OPEN_ATTFILE		"Cannot open the map attribute file. Please re-install Ard|Net."
#define KSTR_CMERR_FAIL_OPEN_ATTFILE		"지정한 맵 특성 파일을 읽어올 수 없습니다. 다시 설치해주시기 바랍니다."

#define CMERR_FAIL_OPEN_MESH				1005
#define ESTR_CMERR_FAIL_OPEN_MESH			"OBSOLUTE FILE FORMAT - Please constct with us."
#define KSTR_CMERR_FAIL_OPEN_MESH			"더이상 유효하지 않는 파일입니다. 만일 이 에러가 발생되었다면 저희에게 연락해주세요."

#define CMERR_FAIL_INIT_CONSOLE				1006
#define ESTR_CMERR_FAIL_INIT_CONSOLE		"Cannot init the console. Please re-install Ard|Net."
#define KSTR_CMERR_FAIL_INIT_CONSOLE		"콘솔 초기화에 실패하였습니다."


#endif


#define CMERR_EXCEED_SIZE					1010
#define ESTR_CMERR_EXCEED_SIZE				"Logical Error: Size is exceed."
#define KSTR_CMERR_EXCEED_SIZE				"크기가 너무 큽니다."


/*	Error Code by Jangho, Lee	*/
#define CMERR_JANGHO_LEE					2000

#define CMERR_CANT_MAP_FILE_TO_MEMORY		2001
#define ESTR_CMERR_CANT_MAP_FILE_TO_MEMORY	"Cannot Map File to Memory."
#define KSTR_CMERR_CANT_MAP_FILE_TO_MEMORY	"파일을 메모리로 매핑할 수 없습니다."

#define CMERR_CANT_CREATE_BITMAP			2002
#define ESTR_CMERR_CANT_CREATE_BITMAP		"Cannot create(convert) bitmap handle."
#define KSTR_CMERR_CANT_CREATE_BITMAP		"비트맵 핸들을 생성(변환)할 수 없습니다."

#define CMERR_CANT_TXZ_BACKUP_FILE			2003
#define ESTR_CMERR_CANT_TXZ_BACKUP_FILE		"Cannot save backup file ( backup.txz )"
#define KSTR_CMERR_CANT_TXZ_BACKUP_FILE		"아래 Second Error Message의 파일을 backup.txz에 백업할 수 없습니다."

#define CMERR_CANT_OPEN_FOR_SAVE			2004
#define ESTR_CMERR_CANT_OPEN_FOR_SAVE		"Cannot open file to save. File might be used another program."
#define KSTR_CMERR_CANT_OPEN_FOR_SAVE		"파일을 저장하려고 하였으나, 파일을 열 수 없습니다. 이미 사용중일 가능성이 높습니다."

#define CMERR_CANT_WRITE_FILE				2005
#define ESTR_CMERR_CANT_WRITE_FILE			"Cannot write to file."
#define KSTR_CMERR_CANT_WRITE_FILE			"파일에 데이타를 쓸 수 없습니다."

#define CMERR_CANT_READ_FILE				2005
#define ESTR_CMERR_CANT_READ_FILE			"Cannot read from file."
#define KSTR_CMERR_CANT_READ_FILE			"파일의 데이타를 읽을 수 없습니다."

#define CMERR_INVALID_FILE_FORMAT			2006
#define ESTR_CMERR_INVALID_FILE_FORMAT		"Cannot open file. Invalid file format."
#define KSTR_CMERR_INVALID_FILE_FORMAT		"파일을 열 수 없습니다. 파일 형식이 다릅니다."

#define CMERR_PREVIOUS_VERSION				2007
#define ESTR_CMERR_PREVIOUS_VERSION			"Previous vesion."
#define KSTR_CMERR_PREVIOUS_VERSION			"이전 버전입니다."

#define CMERR_NEXT_VERSION					2008
#define ESTR_CMERR_NEXT_VERSION				"Next vesion."
#define KSTR_CMERR_NEXT_VERSION				"이후 버전입니다."

	/* Map|Blast Errors */
#define CMERR_INCORRECT_MAP_ID				2100
#define ESTR_CMERR_INCORRECT_MAP_ID			"Not Map|Blast file. ( *.map )"
#define KSTR_CMERR_INCORRECT_MAP_ID			"Map|Blast ( *.map ) 파일이 아닙니다. 파일 내부 ID가 틀립니다."

/*	Error Code by Jayoung, Na	*/
#define CMERR_JAYOUNG_NA					3000

/*	Error Code by Youngho, Kim	*/
#define CMERR_YOUNGHO_KIM					4000

#define CMERR_WINSOCK_NOT_AVAIL				4001
#define ESTR_CMERR_WINSOCK_NOT_AVAIL	"Winsock not available."
#define KSTR_CMERR_WINSOCK_NOT_AVAIL	"Winsock이 사용불능입니다."

#define CMERR_CANT_CREATE_SOCKET			4002
#define ESTR_CMERR_CANT_CREATE_SOCKET	"Can't create socket."
#define KSTR_CMERR_CANT_CREATE_SOCKET	"소켓을 열 수 없습니다."

#define CMERR_CANT_SO_REUSEADDR				4003
#define ESTR_CMERR_CANT_SO_REUSEADDR	"Can't setsockopt SO_REUSEADDR"
#define KSTR_CMERR_CANT_SO_REUSEADDR	"SO_REUSEADDR설정에 실패했습니다."

#define CMERR_CANT_SO_LINGER				4004
#define ESTR_CMERR_CANT_SO_LINGER		"Can't setsockopt SO_LINGER"
#define KSTR_CMERR_CANT_SO_LINGER		"SO_LINGER설정에 실패했습니다."

#define CMERR_CANT_BIND_SOCKET				4005
#define ESTR_CMERR_CANT_BIND_SOCKET		"Can't bind address."
#define KSTR_CMERR_CANT_BIND_SOCKET		"bind()에 실패했습니다."

#define CMERR_INVALID_ARGUMENT				4006
#define ESTR_CMERR_INVALID_ARGUMENT		"Invalid Argument."
#define KSTR_CMERR_INVALID_ARGUMENT		"인자가 잘못되었습니다."

#define CMERR_SENDTO_FAILED					4007
#define ESTR_CMERR_SENDTO_FAILED		"sendto() failed."
#define KSTR_CMERR_SENDTO_FAILED		"sendto() 에서 실패했습니다."

#define CMERR_CANT_SET_WSA_READ				4008
#define ESTR_CMERR_CANT_SET_WSA_READ	"Can't set WSA_READ Message."
#define KSTR_CMERR_CANT_SET_WSA_READ	"WSA_READ Message 설정에 실패했습니다."

#define CMERR_FILE_READ_ERROR				4009
#define ESTR_CMERR_FILE_READ_ERROR		"Can't read FILE"
#define KSTR_CMERR_FILE_READ_ERROR		"파일을 읽을 수 없습니다."

#define CMERR_FILE_WRITE_ERROR				4010
#define ESTR_CMERR_FILE_WRITE_ERROR		"Can't write FILE"
#define KSTR_CMERR_FILE_WRITE_ERROR		"파일에 저장할 수 없습니다."

#define CMERR_UNKNOWN_USER					4011
#define ESTR_CMERR_UNKNOWN_USER			"Unknown user name."
#define KSTR_CMERR_UNKNOWN_USER			"등록이 안된 유저입니다."

#define CMERR_INVALID_PASSWORD				4012
#define ESTR_CMERR_INVALID_PASSWORD		"Invalid password."
#define KSTR_CMERR_INVALID_PASSWORD		"암호가 틀립니다."


/*******************************
	Blast|3D Error
*******************************/
#define CMERR_CANT_INIT_B3D					5001
#define ESTR_CMERR_CANT_INIT_B3D			"Can't init Blast|3D."
#define KSTR_CMERR_CANT_INIT_B3D			"Blast|3D를 초기화하는데 실패하였습니다."

#define CMERR_B3D_INVALID_DRIVER			5002
#define ESTR_CMERR_B3D_INVALID_DRIVER		"Driver is invalid."
#define KSTR_CMERR_B3D_INVALID_DRIVER		"유효한 드라이버가 존재하지 않습니다."

#define CMERR_OBJ_NOTFOUND_MATERIAL			5002
#define ESTR_CMERR_OBJ_NOTFOUND_MATERIAL	"Material(s) or Texture(s) Not Found."
#define KSTR_CMERR_OBJ_NOTFOUND_MATERIAL	"텍스쳐 혹은 재질이 존재하지 않습니다."

#define CMERR_OBJ_NOTFOUND_BMP			5003
#define ESTR_CMERR_OBJ_NOTFOUND_BMP		"*.BMP Not Found."
#define KSTR_CMERR_OBJ_NOTFOUND_BMP		"비트맵파일이 존재하지 않습니다."

#define CMERR_INCORRECT_FILEFORMAT			5004
#define ESTR_CMERR_INCORRECT_FILEFORMAT		"It's Invalid File or Old Version of RSM"
#define KSTR_CMERR_INCORRECT_FILEFORMAT		"잘못된 파일이거나 오래된버젼입니다."

#define CMERROR_RML_MODIFIED			5005
#define ESTR_CMERROR_RML_MODIFIED		"RML Modified."
#define KSTR_CMERROR_RML_MODIFIED		"RML이 외부에서 수정되었습니다."



#endif	/*	_CMERRORDEF_H	*/

