module codegen
// Original author: Saptak Bhoumik
import parser
fn func_return(name string, ast parser.Ast) (string,[]string,parser.Function){
	required_list:=ast.function_return_type
	mut return_item:=""
	mut function:=parser.Function{}
	mut type_of_return:=[]string{}
	mut free:=[]string{}
	for item in required_list{
		if name == item.name{
			type_of_return=item.return_type
			free=item.free
			function=item
			break
		}
	}
	if type_of_return==[]string{} || type_of_return==["void"]{
		return_item="void * $name ("
	}
	else if type_of_return==["str"]{
		return_item="char * $name ("
	}
	else if type_of_return==["int"]{
		return_item="int64_t $name ("
	}
	else if type_of_return==["float"]{
		return_item="double $name ("
	}
	return return_item,free,function
}
fn func_return_if_else(name string, ast parser.Ast) ([]string){
	required_list:=ast.function_return_type
	mut type_of_return:=[]string{}
	for item in required_list{
		if name == item.name{
			type_of_return=item.return_type
			break
		}
	}
	return type_of_return
}