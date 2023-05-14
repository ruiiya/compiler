# Building a LL(1) Parser for VC
## Thành viên nhóm 8:
- Phạm Nguyễn Tuấn Hoàng (20020015)
- Đỗ Việt Hưng (20020139)

## Cấu trúc code
### Thành phần
- [grammar.dat](grammar.dat): Tệp bao gồm các ngữ pháp đã được chuẩn hóa về LL(1).
- [grammar.h](grammar.h): Tệp bao gồm một lớp giúp đọc và kiểm tra tệp grammar.dat và các lớp để định nghĩa production và rule.
- [lexer.h](lexer.h): Tệp bao gồm một lớp lớp giúp đọc tệp đầu vào và trả lại các token là đầu vào của parser để tiến hành phân tích.
- [main.cpp](main.cpp): Tệp bắt đầu chương trình bao gồm các bước để xử lý bài toán.
- [parser.h](parser.h): Tệp bao gồm một lớp để định nghĩa đối tượng AST và một lớp để tạo và thu gọn đối tượng AST.
- [parsing_table.h](parsing_table.h): Tệp bao gồm một lớp để định nghĩa và tạo parsing table.
- [utils.h](utils.h): Tệp bao gồm các hàm để tính các tập FIRST và FOLLOW.

## Thực nghiệm
### Cài đặt
Ngôn ngữ sử dụng: C++17 (MSYS2 MinGW64).
Chạy lệnh để dịch chương trình.
```
g++ -std=c++17 -g main.cpp -o main 
```
Chạy lệnh để chạy chương trình.
```
main.exe [input_file] [output_file] [grammar_file]
```
**Lưu ý**: Nhập tên file mà không có đuôi (Ví dụ: chỉ nhập "in" thay vì "in.vc, tương tự với "\*.vcps" và "\*.dat").
