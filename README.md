# Building a LL(1) Parser for VC
## Thành viên nhóm 8:
- Phạm Nguyễn Tuấn Hoàng (20020015)
- Đỗ Việt Hưng (20020139)

## Cấu trúc code
### Thành phần
- [main.cpp](main.cpp): nhận đầu vào là file chứa code VC (\*.vc) và đầu ra là file (\*.vctok) chứa danh sách các từ của VC code
- [DatLoader.h](DatLoader.h): chứa các hàm phân tích.
- [Common.h](Common.h): chứa các hàm hỗ trợ (string_format, trim, ... etc).
- [Pattern.h](Pattern.h): Chứa class Pattern dùng để mô tả các pattern (letter, digit, ... etc).
- [DFA.h](DFA.h): Chứa class DFA dùng để mô tả máy automat.
- [Tokenizer.h](Tokenizer.h): chứa class Tokenizer giúp đọc file input đầu vào.
- [transitions.dat](transitions.dat): chứa các danh sách các patterns, keywords, transition table và acceptable ending states.

- [grammar.dat](grammar.dat): Tệp bao gồm các ngữ pháp đã được chuẩn hóa về LL(1).

- [grammar.h](grammar.h): Tệp bao gồm một lớp giúp đọc và kiểm tra tệp grammar.dat và các lớp để định nghĩa production và rule.

- [lexer.h](lexer.h): Tệp bao gồm một lớp lớp giúp đọc tệp đầu vào và trả lại các token là đầu vào của parser để tiến hành phân tích.

- [main.cpp](main.cpp): Tệp bắt đầu chương trình bao gồm các bước để xử lý bài toán.

- [parser.h](parser.h): Tệp bao gồm một lớp để định nghĩa đối tượng AST và một lớp để tạo và thu gọn đối tượng AST.

- [parsing_table.h](parsing_table.h): Tệp bao gồm một lớp để định nghĩa và tạo parsing table.

- [utils.h](utils.h): Tệp bao gồm các hàm để tính các tập FIRST và FOLLOW.

## Thực nghiệm
### Cài đặt
Ngôn ngữ: C++
Ngôn ngữ sử dụng: C++17 (MSYS2 MinGW64)
Chạy lệnh để dịch chương trình.
```
g++ -std=c++17 -g main.cpp -o main 
```
Chạy lệnh 
```
main.exe [input_file] [output_file] [grammar_file]
```
**Lưu ý**: Nhập tên file mà không có đuôi (Ví dụ: chỉ nhập "in" thay vì "in.vc, tương tự với "\*.vcps" và "\*.dat").
