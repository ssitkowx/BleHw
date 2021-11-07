///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Json.h"
#include "LoggerHw.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool Json::add (std::string_view v_key, const double v_value, cJSON * v_node)
{
	if (cJSON_AddNumberToObject (v_node, v_key.data (), (double &)(v_value)) == NULL) { return false; };
	return true;
}

bool Json::add (std::string_view v_key, std::string_view v_value, cJSON * v_node)
{
	if (cJSON_AddStringToObject (v_node, v_key.data (), v_value.data ()) == NULL) { return false; };
	return true;
}
 
void Json::logWarning (std::string_view v_msg, std::string_view v_functionName, std::string_view v_nodeName)
{
	std::string msg;
	msg.append (v_msg);
	msg.append ("Function name: ");
	msg.append (v_functionName);
	msg.append (". Node name: ");
	msg.append (v_nodeName);
	msg.append (".");
	//LOGW (MODULE, msg.c_str ());
}

void Json::isNodeJsonEmpty (std::string_view v_functionName, std::string_view v_nodeName, const cJSON * const v_node)
{
	if (v_node == NULL) { logWarning ("Empty json node. ", v_functionName, v_nodeName); }
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


