# 🚀 Nova Language Compiler

Nova is a custom-designed programming language with a full compiler pipeline implemented in C++.

---

## 📌 Features

- ✔ Lexical Analysis (Tokenizer)
- ✔ Parsing (AST Generation)
- ✔ Semantic Analysis (Type Checking + Symbol Table)
- ✔ Code Generation (to C++)
- ✔ Program Execution
- ✔ Simple GUI for running code

---

## 🧠 Compiler Stages

1. **Lexer**
   - Converts source code into tokens

2. **Parser**
   - Builds Abstract Syntax Tree (AST)

3. **Semantic Analysis**
   - Type checking
   - Scope handling
   - Symbol table management

4. **Code Generation**
   - Converts Nova code into C++ code

5. **Execution**
   - Compiles generated C++ and runs it

---

## 🛠️ Technologies Used

- C++
- Python (for GUI)
- Tkinter
- g++

---

## ▶️ How to Run

### python gui.py
---
## Example Code
---
module launch() {

    text astronaut;
    num fuel;
    
    transmit "Enter astronaut name: ";
    receive astronaut;

    transmit "Enter fuel amount: ";
    receive fuel;

    transmit astronaut + " has " + fuel + " units of fuel.";
    land 0;
}
