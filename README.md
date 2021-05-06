# score-management-console

## 项目简介

基于控制台的简易学生成绩管理系统

## 功能模块

### 1. 显示记录

（1）list：显示当前记录；

（2）list a：显示所有记录；

（3）list x 学号值：显示指定学号记录，并不改变当前记录；

（4）list n 姓名值：显示所有“姓名=”的记录，并不改变当前记录；

### 2. 增加学生记录

（1）add：根据提示输入；

（2）add a：学号 姓名 分数 1 分数 2 分数 3；

### 3. 删除学生记录

（1）rem：删除当前记录；

（2）rem a：删除所有记录；

（3）rem x 学号值：删除指定学号对应记录；

### 4. 修改当前记录

（1）edit：修改当前记录，需要输入所有项；

（2）edit a 学号值 姓名值 分数 1 值 分数 2 值 分数 3 值：修改当前记录所有项；

（3）edit x  学号值：只修改当前记录学号；

（4）edit n 姓名值：只修改当前记录姓名；

（5）edit s1 分数 1 值：只修改当前记录分数 1；

（6）edit s2 分数 2 值：只修改当前记录分数 2；

（7）edit s3 分数 3 值：只修改当前记录分数 3；

### 5. 设置当前记录；

（1）set 序号值：设置当前记录为序号所在的记录并显示；

### 6 排序学生记录

（1）sort：根据平均分排序；

（2）sort x：根据学号排序；

（3）sort n：根据姓名排序；

（4）sort s1：根据分数 1 排序；

（5）sort s2：根据分数 2 排序；

（6）sort s3：根据分数 3 排序；

### 7. 帮助

（1）help：一级命令帮助,列出一级命令名字：list,add,rem,edit,set,sort;

（2）help 一级命令值：进一步列出二级命令选项并解释；

### 8. 退出
