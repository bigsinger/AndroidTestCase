
#pragma once

#define TAG							"[BIGSING::TESTCASE]"
#define Java_Interface_Class_Name	"com/bigsing/NativeHandler"
#define Native_Method_1_Name		"getStr"
#define Native_Method_1_Signature	"(Landroid/content/Context;ILjava/lang/String;)Ljava/lang/String;"
#define Native_Method_2_Name		"getInt"
#define Native_Method_2_Signature	"(Landroid/content/Context;ILjava/lang/String;)I"
#define Native_Method_3_Name		"Jump"
#define Native_Method_3_Signature	"(I[Ljava/lang/Object;)Ljava/lang/Object;"


enum {
	CMD_INIT			=0,
	CMD_GET_TEST_STR	=1,
	CMD_GET_FILE_TEXT	=2,
	CMD_GET_MAC			=3,
	CMD_GET_PPID		=0x991D,
};

namespace Constant{

}