#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

// 自定义学生类
class Student {
public:
	int no = -1; // 序号
	string id = ""; // 学号
	string name = ""; // 姓名
	double s1 = -1; // 分数1
	double s2 = -1; // 分数2
	double s3 = -1; // 分数3
	double avg = -1; // 平均分
};

// 全局变量
const char binDBUrl[] = "./Resource/database/database.bin"; // bin数据库地址
const char txtDBUrl[] = "./Resource/database/database.txt"; // txt数据库地址
vector<Student> sList; // 从数据库获取的学生数据列表
int curIndex = -1; // 记录当前序号
string curId = ""; // 记录当前学号

/* 显示主菜单 */
void menu() {
	cout << "主菜单：" << endl;
	cout << "------------------" << endl;
	cout << "1. 显示学生记录(list)" << endl;
	cout << "2. 增加学生记录(add)" << endl;
	cout << "3. 删除学生记录(rem)" << endl;
	cout << "4. 修改学生记录(edit)" << endl;
	cout << "5. 设置当前记录(set)" << endl;
	cout << "6. 排序学生记录(sort)" << endl;
	cout << "7. 帮助(help)" << endl;
	cout << "8. 退出(quit)" << endl;
	cout << "------------------" << endl;
}



/********
**
**（BEGIN）工具函数
**
********/

/* 判断字符串是否为浮点数 */
bool isDouble(string s) {
	if (s.empty()) return false;
	if (s.size() == 1 && !isdigit(s[0])) return false;
	if (s[0] != '-' && s[0] != '.' && !isdigit(s[0])) return false;
	if (s.size() == 2 && s[0] == '-' && s[1] == '.') return false;
	int dotCount = 0;
	if (s[0] == '.') dotCount++;
	if (s.size() >= 2)
		for (int i = 1; i < s.size(); i++) {
			if (s[i] == '.') dotCount++;
			else if (!isdigit(s[i])) return false;
		}
	if (dotCount >= 2) return false;
	return true;
}

/* 判断字符串是否为整数 */
bool isInt(string s) {
	if (s.empty()) return false;
	if (s.size() == 1 && !isdigit(s[0])) return false;
	if (s[0] != '-' && !isdigit(s[0])) return false;
	if (s.size() >= 2)
		for (int i = 1; i < s.size(); i++) {
			if (!isdigit(s[i])) return false;
		}
	return true;
}

/* 为命令数组补充输入额外的参数 */
/* args：原参数列表，cmdLen：命令长度，paraLen：需要参数长度，msg：提示信息 */
vector<string> reqPara(vector<string> args, int cmdLen, int paraLen, string msg) {
	if (args.size() < cmdLen + paraLen) {
		cout << msg;
		string paraLine, temp;
		getline(cin, paraLine);
		stringstream input(paraLine);
		while (input >> temp)
			args.push_back(temp);
	}
	return args;
}

/* 检查参数列表中含有的学生信息是否合法 */
/* 检查的参数依次为 学号 姓名 成绩1 成绩2 成绩3，begin: args中参数开始的下标 */
/* 学号姓名不能为空，成绩必须为大于等于0的浮点数 */
bool checkPara(vector<string> args, int begin) {
	string id = args[begin];
	string name = args[begin + 1];
	string s1Str = args[begin + 2];
	string s2Str = args[begin + 3];
	string s3Str = args[begin + 4];
	if (id == "" || name == "") return false;
	if (!isDouble(s1Str) || !isDouble(s2Str) || !isDouble(s3Str)) return false;
	double s1 = strtod(s1Str.c_str(), NULL);
	double s2 = strtod(s2Str.c_str(), NULL);
	double s3 = strtod(s3Str.c_str(), NULL);
	if (s1 < 0 || s2 < 0 || s3 < 0) return false;
	return true;
}

/* 判断学号是否存在 */
bool idExist(string id) {
	for (Student s : sList)
		if (id == s.id) return true;
	return false;
}

/* 判断是否存在学生数据 */
bool dataExist() {
	if (sList.empty()) {
		cout << "没有记录！" << endl;
		return false;
	}
	return true;
}

/* 数据变更后重新编号 */
void renumber() {
	int index = 0;
	// 注意：这里不能用foreach循环，否则遍历会出现异常！
	for (int i = 0; i < sList.size(); i++)
		sList[i].no = ++index;
}

/* 数据变更后自动调整当前指针 */
void setCur() {
	// 清空数据时重置指针
	if (sList.empty()) {
		curIndex = -1;
		curId = "";
		return;
	}
	// sList非空才会执行下面的逻辑
	// 初始化时设置默认指针
	if (curIndex == -1) {
		curIndex = sList[0].no;
		curId = sList[0].id;
		return;
	}
	// 删除操作后指针越界时移动指针
	if (curIndex > sList.size()) {
		curIndex = sList.size();
		curId = sList[curIndex - 1].id;
		return;
	}
	// 进行set设置及当前指针记录被删除后移动指针
	if (curId != sList[curIndex - 1].id) {
		curId = sList[curIndex - 1].id;
	}
}

/* 数据变更后寻找原学生对象的新序号 */
void setCurIndex() {
	for (int i = 0; i < sList.size(); i++) {
		if (curId == sList[i].id) {
			curIndex = sList[i].no;
			break;
		}
	}
}

/* 打印数据序列 */
void print(vector<Student> list) {

	if (list.empty()) return;

	cout << "=======================================================================" << endl;
	cout << "||  序号 |      学号|      姓名|   成绩1|   成绩2|   成绩3|  平均成绩||" << endl;
	cout << "-----------------------------------------------------------------------" << endl;

	// 格式化输出数据
	for (Student s : list) {
		cout << "|| " << (s.id == curId ? "* " : "  ") << (s.no < 10 ? "0" : "") << s.no << "  |";
		cout << setw(10) << s.id << "|" << setw(10) << s.name << "|";
		cout << fixed << setprecision(2) << setw(8) << s.s1 << "|" << setw(8) << s.s2 << "|" << setw(8) << s.s3 << "|";
		cout << fixed << setprecision(2) << setw(10) << s.avg << "||" << endl;
	}

	cout << "=======================================================================" << endl;

}

/* 比较函数 */
/* 以下全为升序排序 */
bool cmpByAvg(Student a, Student b) {
	return a.avg < b.avg;
}
bool cmpById(Student a, Student b) {
	return a.id < b.id;
}
bool cmpByName(Student a, Student b) {
	return a.name < b.name;
}
bool cmpByS1(Student a, Student b) {
	return a.s1 < b.s1;
}
bool cmpByS2(Student a, Student b) {
	return a.s2 < b.s2;
}
bool cmpByS3(Student a, Student b) {
	return a.s3 < b.s3;
}

/********
**
**（END）工具函数
**
********/



/********
**
**（BEGIN）数据库操作相关函数
**
** 注意用户是否有C盘写入权限
** bin数据库格式：“学号 姓名 成绩1 成绩2 成绩3 平均成绩”，一行表示一条记录
** 示例：08	yu	98	78	76	84
**
********/

/* 检查学生对象完整性约束 */
/* 序号必须为大于0的整数，学号姓名不能为空，成绩必须为大于等于0的浮点数，平均成绩计算误差不超过0.05 */
bool constraints(Student s) {
	if (s.no <= 0) return false;
	if (s.id == "" || s.name == "") return false;
	if (s.s1 < 0 || s.s2 < 0 || s.s3 < 0) return false;
	double avg = (s.s1 + s.s2 + s.s3) / 3.0;
	if (abs(s.avg - avg) > 0.05) return false;
	return true;
}

/* 持久化 */
void persistence() {

	ofstream binDB, txtDB;

	// 打开数据库文件并清空
	binDB.open(binDBUrl, ios::trunc | ios::binary);
	txtDB.open(txtDBUrl, ios::trunc);

	// 将新数据写入数据库
	txtDB << "记录个数：" << endl;
	txtDB << sList.size() << endl;
	txtDB << "\n序号\t学号\t姓名\t成绩1\t成绩2\t成绩3\t平均成绩" << endl;

	int index = 0; // 序号
	for (Student s : sList) {
		s.no = ++index;
		binDB << s.id << "\t" << s.name << "\t" << s.s1 << "\t" << s.s2 << "\t" << s.s3 << "\t" << s.avg << endl;
		txtDB << s.no << "\t" << s.id << "\t" << s.name << "\t" << fixed << setprecision(2) << s.s1 << "\t" << s.s2 << "\t" << s.s3 << "\t" << s.avg << endl;
	}

	binDB.close();
	txtDB.close();

}

/* 新建数据库 */
void createDB(const char url[]) {
	ofstream createDB;
	createDB.open(url, ios::binary);
	if (!createDB) {
		cout << "数据库“" << url << "”不存在，且自动创建失败\n请检查用户是否有C盘写入权限！" << endl;
		system("pause");
		exit(0);
	}
	else cout << "数据库“" << url << "”不存在，已自动创建！" << endl;
	createDB.close();
}

/* 初始化数据库 */
void DBUtil() {

	ifstream binDB, txtDB;

	// 打开数据库文件
	binDB.open(binDBUrl, ios::binary);
	txtDB.open(txtDBUrl);

	// 数据库不存在则新建数据库
	if (!binDB) {
		createDB(binDBUrl);
		binDB.open(binDBUrl, ios::binary);
	}
	if (!txtDB) {
		createDB(txtDBUrl);
		txtDB.open(txtDBUrl);
	}

	// 将数据库的数据读取进程序
	int no = 0;
	while (!binDB.eof()) {
		string dataLine; // 数据行，临时字符串
		Student student;
		getline(binDB, dataLine);
		stringstream input(dataLine);
		if (dataLine != "") {
			student.no = ++no;
			input >> student.id >> student.name >> student.s1 >> student.s2 >> student.s3 >> student.avg;
			if (constraints(student))
				sList.push_back(student);
			else {
				cout << "数据库文件损坏或数据异常，请检查数据库中数据的完整性！" << endl;
				system("pause");
				exit(0);
			}
		}
	}
	binDB.close();
	txtDB.close();

	persistence(); // 保证txt数据库内容与bin数据库一致

}

/********
**
**（END）数据库操作相关函数
**
********/



/********
**
**（BEGIN）1、list操作命令相关函数
**
********/

/* 打印查询信息 */
void printQuery(vector<string> args, vector<Student> res) {

	print(res);

	if (args.size() <= 1) return;

	if (args[1] == "a") {
		cout << "\t共有记录数目：" << res.size() << endl;
		cout << "\t当前记录序号：" << curIndex << endl;
	}
	else if (args[1] == "n") {
		cout << "查到名字为“" << args[2] << "”的记录 " << res.size() << " 条" << endl;
	}
	else if (args[1] == "x")
		if (res.size() == 0) cout << "输入学号不存在！" << endl;

}

/* 查询当前记录 */
vector<Student> queryCur() {
	vector<Student> res;
	for (Student s : sList) {
		if (curId == s.id) {
			res.push_back(s);
			break;
		}
	}
	return res;
}

/* 根据学号查询学生 */
vector<Student> queryById(string id) {
	vector<Student> res;
	for (Student s : sList) {
		if (id == s.id) {
			res.push_back(s);
			break;
		}
	}
	return res;
}

/* 根据姓名查询学生 */
vector<Student> queryByName(string name) {
	vector<Student> res;
	for (Student s : sList)
		if (name == s.name) res.push_back(s);
	return res;
}

/* 1. 显示学生记录 */
void list(vector<string> args) {
	vector<Student> res;
	// list：显示当前记录
	if (args.size() <= 1) {
		if (!dataExist()) return;
		res = queryCur();
	}
	else {
		// list a：显示所有记录
		if (args[1] == "a") {
			if (!dataExist()) return;
			res = sList;
		}
		// list x 学号值：显示指定学号记录，并不改变当前记录
		else if (args[1] == "x") {
			// if (!dataExist()) return; // 按照参考程序，这里没有进行数据为空判断
			args = reqPara(args, 2, 1, "请输入学号：");
			if (args.size() < 3) return; // 未输入学号则返回控制台
			res = queryById(args[2]);
		}
		// list n 姓名值：显示所有“姓名=”的记录，并不改变当前记录
		else if (args[1] == "n") {
			if (!dataExist()) return;
			args = reqPara(args, 2, 1, "请输入姓名：");
			if (args.size() < 3) return; // 未输入姓名则返回控制台
			res = queryByName(args[2]);
		}
		// list 参数错误
		else {
			cout << "命令参数不正确！" << endl;
			return;
		}
	}
	printQuery(args, res);
}

/********
**
**（END）1、list操作命令相关函数
**
********/



/********
**
**（BEGIN）2、add操作命令相关函数
**
********/

/* 打印添加信息 */
void printAdd() {
	cout << "增加了如下记录：" << endl;
	// 执行一次list，打印添加信息
	list({ "list" });
}

/* 插入学生记录 */
/* begin：args中参数开始的下标 */
bool insert(vector<string> args, int begin) {

	// 输入合法则创建新的学生对象
	Student s;
	s.no = sList.size() + 1;
	s.id = args[begin];
	s.name = args[begin + 1];
	s.s1 = strtod(args[begin + 2].c_str(), NULL);
	s.s2 = strtod(args[begin + 3].c_str(), NULL);
	s.s3 = strtod(args[begin + 4].c_str(), NULL);
	s.avg = (s.s1 + s.s2 + s.s3) / 3.0;

	// 将新增学生对象存入程序和数据库，并设置为当前对象
	sList.push_back(s);
	curIndex = s.no;
	curId = s.id;
	persistence();

	return true;

}

/* 2. 增加学生记录 */
void add(vector<string> args) {

	// add 根据提示输入
	if (args.size() <= 1) {
		args = reqPara(args, 1, 5, "请输入记录：");
		if (args.size() < 6 || !checkPara(args, 1)) {
			cout << "输入错误！记录格式为：学号 姓名 成绩1 成绩2 成绩3" << endl;
			return;
		}
		else {
			if (idExist(args[1])) {
				cout << "输入学号已存在，不可以重新输入！" << endl;
				return;
			}
			insert(args, 1);
		}
	}
	else {
		// add a 学号 姓名 分数1 分数2 分数3
		if (args[1] == "a") {
			if (args.size() < 7 || !checkPara(args, 2)) {
				cout << "“add a”命令参数格式为：add a 学号 姓名 成绩1 成绩2 成绩3" << endl;
				return;
			}
			if (idExist(args[2])) {
				cout << "输入学号已存在，不可以重新输入！" << endl;
				return;
			}
			insert(args, 2);
		}
		// add 错误参数
		else {
			cout << "命令参数不正确！" << endl;
			return;
		}
	}

	printAdd();

}

/********
**
**（END）2、add操作命令相关函数
**
********/



/********
**
**（BEGIN）3、rem操作命令相关函数
**
** 在演示程序中，该部分的命令提示存在混乱和不一致
** 这里对提示进行了一些修改和补充，保证提示的统一和交互的友好
**
********/

/* 确认删除信息 */
bool confirmRm(string msg) {
	cout << msg;
	char cfm = getchar();
	// 清空输入缓存
	string garbage;
	getline(cin, garbage);
	// 根据参考程序，只有'y*'被判定为确认
	if (cfm == 'y') return true;
	return false;
}

/* 根据学号删除学生 */
void rmById(string id) {
	if (!dataExist()) return;
	// 根据学号查找学生的下标
	int index = -1;
	for (int i = 0; i < sList.size(); i++) {
		if (id == sList[i].id) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		cout << "学生不存在！" << endl;
		return;
	}
	cout << "你要删除的学生信息为：" << endl;
	// 执行一次list x，打印要删除的学生信息
	list({ "list", "x", id });
	if (confirmRm("你确定要删除吗？(y/n)：")) {
		sList.erase(sList.begin() + index); // 从程序数据中删除对应学生
		renumber(); // 重新编号
		persistence(); // 持久化
		setCur(); // 重设指针
		cout << "删除成功！" << endl;
	}
	else cout << "删除失败！" << endl;
}

/* 删除全部学生 */
void rmAll() {
	if (!dataExist()) return;
	if (confirmRm("你确定要删除所有记录吗？(y/n)：")) {
		sList.clear();
		setCur();
		persistence();
		cout << "删除成功！" << endl;
	}
	else cout << "删除失败！" << endl;
}

/* 3. 删除学生记录 */
void remove(vector<string> args) {

	// rem：删除当前记录
	if (args.size() <= 1) rmById(curId);
	else {
		// rem a：删除所有记录
		if (args[1] == "a") rmAll();
		// rem x 学号值：删除指定学号对应记录
		else if (args[1] == "x") {
			if (!dataExist()) return;
			args = reqPara(args, 2, 1, "请输入要删除的学生的学号：");
			if (args.size() < 3) return;
			rmById(args[2]);
		}
		// rem 错误参数
		else {
			cout << "命令参数不正确！" << endl;
			return;
		}
	}

}

/********
**
**（END）3、rem操作命令相关函数
**
********/



/********
**
**（BEGIN）4、edit操作命令相关函数
**
** 在演示程序中，该部分的命令提示存在错误
** 这里对提示进行了修正
**
********/

/* 打印编辑信息 */
void printUpdate(int flag) {
	cout << "修改" << (flag == 0 ? "前" : "后") << "记录为:" << endl;
	// 执行一次list
	list({ "list" });
}

/* 更新学生信息 */
/* begin：args中参数开始的下标 opt：需要修改的参数位置（其中0表示修改全部参数） */
void update(vector<string> args, int begin, int opt) {

	if (opt == 0) {
		sList[curIndex - 1].id = args[begin];
		curId = args[begin];
		sList[curIndex - 1].name = args[begin + 1];
		sList[curIndex - 1].s1 = strtod(args[begin + 2].c_str(), NULL);
		sList[curIndex - 1].s2 = strtod(args[begin + 3].c_str(), NULL);
		sList[curIndex - 1].s3 = strtod(args[begin + 4].c_str(), NULL);
		sList[curIndex - 1].avg = (sList[curIndex - 1].s1 + sList[curIndex - 1].s2 + sList[curIndex - 1].s3) / 3.0;
	}
	else if (opt == 1) {
		sList[curIndex - 1].id = args[begin];
		curId = args[begin];
	}
	else if (opt == 2) sList[curIndex - 1].name = args[begin];
	else if (opt == 3) sList[curIndex - 1].s1 = strtod(args[begin].c_str(), NULL);
	else if (opt == 4) sList[curIndex - 1].s2 = strtod(args[begin].c_str(), NULL);
	else if (opt == 5) sList[curIndex - 1].s3 = strtod(args[begin].c_str(), NULL);
	if (opt == 3 || opt == 4 || opt == 5) sList[curIndex - 1].avg = (sList[curIndex - 1].s1 + sList[curIndex - 1].s2 + sList[curIndex - 1].s3) / 3.0;

	persistence();

}

/* 修改当前学生的全部信息 */
/* begin: args中参数开始的下标 */
void editAll(vector<string> args, int begin) {
	if (!dataExist()) return;
	if (begin == 2 && (args.size() < 7 || !checkPara(args, begin))) {
		cout << "“edit a”命令参数格式为：edit a 学号 姓名 成绩1 成绩2 成绩3" << endl;
		return;
	}
	printUpdate(0);
	args = reqPara(args, begin, 5, "请输入记录：");
	if (begin == 1 && (args.size() < 6 || !checkPara(args, begin))) {
		cout << "输入错误！记录格式为：学号 姓名 成绩1 成绩2 成绩3" << endl;
		return;
	}
	else {
		if (idExist(args[begin]) && args[begin] != curId) {
			cout << "输入学号已存在，不能有重复学号！" << endl; // 学号不与其他学号重复，但可继续为当前学号
			return;
		}
		update(args, begin, 0);
		printUpdate(1);
	}
}

/* 修改当前学生的学号 */
void editId(vector<string> args) {
	if (!dataExist()) return;
	printUpdate(0);
	args = reqPara(args, 2, 1, "请输入学号新值：");
	if (args.size() < 3) return;
	if (idExist(args[2]) && args[2] != curId) {
		cout << "输入学号已存在，不能修改！" << endl;
		return;
	}
	update(args, 2, 1);
	printUpdate(1);
}

/* 修改当前学生的姓名 */
void editName(vector<string> args) {
	if (!dataExist()) return;
	printUpdate(0);
	args = reqPara(args, 2, 1, "请输入姓名新值：");
	if (args.size() < 3) return;
	if (args[2] != "") {
		update(args, 2, 2);
		printUpdate(1);
	}
}

/* 修改当前学生的分数 */
void editScore(vector<string> args) {
	if (!dataExist()) return;

	int opt = 0;
	if (args[1] == "s1") opt = 1;
	else if (args[1] == "s2") opt = 2;
	else opt = 3; // args[1] == "s3"

	printUpdate(0);
	args = reqPara(args, 2, 1, "请输入分数" + to_string(opt) + "新值：");
	if (args.size() < 3) return;
	if (isDouble(args[2])) {
		update(args, 2, opt + 2);
		printUpdate(1);
	}
	else cout << "输入信息有误！未更新。" << endl;
}

/* 4. 修改学生记录 */
void edit(vector<string> args) {
	// edit：修改当前记录，需要输入所有项
	if (args.size() <= 1) editAll(args, 1);
	else {
		// edit a 学号值 姓名值 分数 1 值 分数 2 值 分数 3 值：修改当前记录所有项
		if (args[1] == "a") editAll(args, 2);
		// edit x 学号值：只修改当前记录学号
		else if (args[1] == "x") editId(args);
		// edit n 姓名值：只修改当前记录姓名
		else if (args[1] == "n") editName(args);
		// edit s1 分数 1 值：只修改当前记录分数 1
		// edit s2 分数 2 值：只修改当前记录分数 2
		// edit s3 分数 3 值：只修改当前记录分数 3
		else if (args[1] == "s1" || args[1] == "s2" || args[1] == "s3") editScore(args);
		// edit 错误参数
		else cout << "命令参数不正确！" << endl;
	}
}

/********
**
**（END）4、edit操作命令相关函数
**
********/



/********
**
**（BEGIN）5、set操作命令相关函数
**
** 在演示程序中，该部分存在参数判定错误，这里进行了修正
**
** 在演示程序中，该部分的命令提示存在混乱和不一致
** 这里对提示进行了一些修改，保证提示的统一和交互的友好
**
********/

/* 5. 设置当前记录 */
void set(vector<string> args) {

	// set 参数不全
	if (args.size() <= 1) {
		cout << "输入错误！“set”命令参数格式为：set 序号值" << endl;
		return;
	}

	// set 序号值：设置当前记录为序号所在的记录并显示
	// 判断是否有数据
	if (!dataExist()) return;

	// 验证参数是否为整数
	if (!isInt(args[1])) {
		cout << "参数格式不正确！" << endl;
		return;
	}

	int newNo = atoi(args[1].c_str());
	// 验证序号是否越界
	if (newNo <= 0 || newNo > sList.size()) {
		cout << "输入的序号不在范围内！" << endl;
		return;
	}

	curIndex = newNo;
	setCur();
	cout << "设置成功！" << endl;

	// 执行一次list，打印当前指向对象
	list({ "list" });

}

/********
**
**（END）5、set操作命令相关函数
**
********/



/********
**
**（BEGIN）6、sort操作命令相关函数
**
********/

/* 6. 排序学生记录 */
void sort(vector<string> args) {

	// sort：根据平均分排序
	if (args.size() <= 1) {
		if (!dataExist()) return;
		sort(sList.begin(), sList.end(), cmpByAvg);
	}
	else {
		// sort x：根据学号排序
		if (args[1] == "x") {
			if (!dataExist()) return;
			sort(sList.begin(), sList.end(), cmpById);
		}
		// sort n：根据姓名排序
		else if (args[1] == "n") {
			if (!dataExist()) return;
			sort(sList.begin(), sList.end(), cmpByName);
		}
		// sort s1：根据分数 1 排序
		else if (args[1] == "s1") {
			if (!dataExist()) return;
			sort(sList.begin(), sList.end(), cmpByS1);
		}
		// sort s2：根据分数 2 排序
		else if (args[1] == "s2") {
			if (!dataExist()) return;
			sort(sList.begin(), sList.end(), cmpByS2);
		}
		// sort s3：根据分数 3 排序
		else if (args[1] == "s3") {
			if (!dataExist()) return;
			sort(sList.begin(), sList.end(), cmpByS3);
		}
		// sort 参数错误
		else {
			cout << "命令参数不正确！" << endl;
			return;
		}
	}

	renumber(); // 重新编号
	setCurIndex(); // 寻找原指针对应的新序号
	persistence(); // 持久化（根据参考程序，排序会影响数据库数据）
	setCur(); // 重设指针

	// 执行一次list a
	list({ "list", "a" });

}

/********
**
**（END）6、sort操作命令相关函数
**
********/



/********
**
**（BEGIN）7、help操作命令相关函数
**
********/

/* 7. 帮助 */
void help(vector<string> args) {

	if (args.size() <= 1) {
		menu();
		return;
	}

	if (args[1] == "list") {
		cout << "\tlist---------------显示当前记录" << endl;
		cout << "\tlist a-------------显示所有记录" << endl;
		cout << "\tlist x 学号值------显示相应“学号”记录" << endl;
		cout << "\tlist n 姓名值------显示所有相应“姓名”记录" << endl;
	}
	else if (args[1] == "add") {
		cout << "\tadd----------------------------------在最后插入一条新记录" << endl;
		cout << "\tadd a 学号 姓名 分数1 分数2 分数3----在最后插入一条新记录（快速）" << endl;
	}
	else if (args[1] == "rem") {
		cout << "\trem----------------删除当前记录" << endl;
		cout << "\trem a--------------删除所有记录" << endl;
		cout << "\trem x 学号值-------删除指定“学号”记录" << endl;
	}
	else if (args[1] == "edit") {
		cout << "\tedit---------------更新当前记录所有项" << endl;
		cout << "\tedit x  学号值-----更新当前记录学号值" << endl;
		cout << "\tedit n  姓名值-----更新当前记录姓名值" << endl;
		cout << "\tedit s1 分数1值----更新当前记录分数1值" << endl;
		cout << "\tedit s2 分数2值----更新当前记录分数2值" << endl;
		cout << "\tedit s3 分数3值----更新当前记录分数3值" << endl;
		cout << "\tedit a 学号 姓名 分数1 分数2 分数3----更新当前记录所有项（快速）" << endl;
	}
	else if (args[1] == "set") {
		cout << "\tset 序号值---------使用“序号”进行定位，并设置为当前记录" << endl;
	}
	else if (args[1] == "sort") {
		cout << "\tsort---------------使用“平均成绩”进行排序" << endl;
		cout << "\tsort x-------------使用“学号”进行排序" << endl;
		cout << "\tsort n-------------使用“姓名”进行排序" << endl;
		cout << "\tsort s1------------使用“分数1”进行排序" << endl;
		cout << "\tsort s2------------使用“分数2”进行排序" << endl;
		cout << "\tsort s3------------使用“分数3”进行排序" << endl;
	}
	else
		cout << "没有“" << args[1] << "”的帮助。" << endl;

}

/********
**
**（END）7、help操作命令相关函数
**
********/



/* 处理操作命令入口 */
void cmd(vector<string> args) {

	if (args.size() <= 0)
		return;

	if (args[0] == "list")
		list(args);
	else if (args[0] == "add")
		add(args);
	else if (args[0] == "rem")
		remove(args);
	else if (args[0] == "edit")
		edit(args);
	else if (args[0] == "set")
		set(args);
	else if (args[0] == "sort")
		sort(args);
	else if (args[0] == "help")
		help(args);
	else if (args[0] == "quit")
		exit(0);
	else if (args[0] == "clear") // 测试用，清屏
		system("cls");
	else
		cout << "非系统命令！" << endl;

}

/* main函数，程序入口 */
int main() {

	system("title 学生成绩管理系统"); // 设置标题
	system("color 4E"); // 设置背景和字体颜色：红色（4）、淡黄色（E）

	DBUtil(); // 初始化数据库，将数据读入程序
	setCur(); // 初始化指针
	menu();	// 显示主菜单

	// 开始接收操作命令
	while (true) {

		cout << ">>:";

		string cmdLine, buffer; // 操作命令行，缓冲字符串
		vector<string> cmdArgs; // 操作命令

		cin.clear();
		getline(cin, cmdLine);
		stringstream input(cmdLine);
		while (input >> buffer)
			cmdArgs.push_back(buffer);

		// 处理操作命令
		cmd(cmdArgs);

	}

	return 0;

}
