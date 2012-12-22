#include "Expression.h"
#include <stack>

using namespace std;
using namespace StormSQL::SQL::Expressions;

ExpressionParser::ExpressionParser(Lexer& _lex, const hash_map<string, OperationInfo>& _ops)
	: lex(&_lex), ops(_ops)
{
}

queue<Token> ExpressionParser::GetRPN()
{
	queue<Token> out;
	stack<Token> operationStack;

	while (!lex->endOfStream())
	{
		Token op;
		Token t = lex->NextToken(false);
		
		switch (t.type)
		{
		case TokenType::IntValue:
		case TokenType::StringValue:
		case TokenType::Identifier:
			out.push(t);
			break;

		case TokenType::Keyword:

			// Function or operator
			if (ops.find(t.strData) != ops.end())
			{
				if (ops[t.strData].isFunction)
				{
					// Function name (next token should be '(')
					Token temp = lex->NextToken(TokenType::Parenthesis, "(");
					lex->PutBackToken(temp);
				}
				else
				{
					while (!operationStack.empty() && ( 
						(ops[t.strData].leftAssoc && ops[t.strData].priority <= ops[operationStack.top().strData].priority) 
								|| 
						(ops[t.strData].priority < ops[operationStack.top().strData].priority) 
							) )
					{
						out.push(operationStack.top());
						operationStack.pop();
					}
				}

				operationStack.push(t);
			}
			else
			{
				// Value or constant
				out.push(t);
			}

			break;

		case TokenType::Separator:

			if (operationStack.empty())
				throw InvalidTokenException(t);

			op = operationStack.top();
			while (op.strData != "(")
			{
				operationStack.pop();
				out.push(op);
				
				if (operationStack.empty())
					throw InvalidTokenException(t);

				op = operationStack.top();
			}

			break;

		case TokenType::Operator:
			
			if (ops.find(t.strData) == ops.end())
				throw InvalidTokenException(t);

			while (!operationStack.empty() && ( 
				(ops[t.strData].leftAssoc && ops[t.strData].priority <= ops[operationStack.top().strData].priority) 
						|| 
				(ops[t.strData].priority < ops[operationStack.top().strData].priority) 
					) )
			{
				out.push(operationStack.top());
				operationStack.pop();
			}

			operationStack.push(t);
			
			break;

		case TokenType::Parenthesis:
			
			if (t.strData == "(")
			{
				operationStack.push(t);
			}
			else 
			{
				if (operationStack.empty())
				throw InvalidTokenException(t);

				op = operationStack.top();
				while (op.strData != "(")
				{
					operationStack.pop();
					out.push(op);
				
					if (operationStack.empty())
						throw InvalidTokenException(t);

					op = operationStack.top();
				}

				// Pop (
				operationStack.pop();

				if (!operationStack.empty() && ops.find(operationStack.top().strData) != ops.end())
				{
					// Token at top of stack is function name
					out.push(operationStack.top());
					operationStack.pop();
				}
			}

			break;

		default:
			throw InvalidTokenException(t);
		}
	}

	while (!operationStack.empty())
	{
		if (operationStack.top().type == TokenType::Parenthesis && operationStack.top().strData == "(")
			throw LexerException("Mismatched parenthesis");

		out.push(operationStack.top());
		operationStack.pop();
	}

	return out;
}

string ExpressionParser::Implode(queue<Token>& q)
{
	stringstream str;
	
	bool first = true;
	while (!q.empty())
	{
		if (!first)
			str << " ";
		first = false;

		switch (q.front().type)
		{
		case TokenType::Identifier:
			str << '`' << q.front().strData << '`';
			break;
		case TokenType::StringValue:
			str << '\'' << q.front().strData << '\'';
			break;
		default:
			str << q.front().strData;
		}

		q.pop();
	}

	return str.str();
}

void ExpressionParser::InsertOp(Token t, stack<Expression*>& values)
{
	OperationInfo op = ops[t.strData];
	vector<Expression*> operands;

	for (int i = 0; i < op.arguments; i++)
	{
		if (values.empty())
			throw InvalidNumberOfArguments(t.strData);

		operands.push_back(values.top());
		values.pop();
	}

	values.push(new CompExpression(operands, op.op)); // oppan gangnam style!

	// CompExpression copies the operands
	for (int i = 0; i < operands.size(); i++)
	{
		delete operands[i];
	}
}

Expression* ExpressionParser::Parse()
{
	queue<Token> rpn = GetRPN();
	stack<Expression*> values;

	while (!rpn.empty())
	{
		Token t = rpn.front();
		rpn.pop();

		switch (t.type)
		{
		case TokenType::IntValue:
			// Integer
			values.push(new ConstExpression(Value(t.longIntData)));
			break;
		case TokenType::StringValue:
			// String
			values.push(new ConstExpression(Value(t.strData)));
			break;
		case TokenType::Identifier:
			// Identifier
			values.push(new VarExpression(t.strData));
		case TokenType::Keyword:
			// Function (operator) or Identifier
			if (ops.find(t.strData) != ops.end())
			{
				// Function or operator
				InsertOp(t, values);
			}
			else
			{
				// Constant or Identifier
				string low = t.strData;
				lex->toLower(low);

				if (low == "true")
					values.push(new ConstExpression(Value(1)));
				else if (low == "false")
					values.push(new ConstExpression(Value(0)));
				else
				{
					// Identifier
					values.push(new VarExpression(t.strData));
				}
			}
			break;
		case TokenType::Operator:
			// Operator
			if (ops.find(t.strData) == ops.end())
				throw InvalidTokenException(t);

			InsertOp(t, values);

			break;

		default:
			throw InvalidTokenException(t);
		}
	}

	Expression* exp = values.top();
	values.pop();

	if (!values.empty())
		throw Exception("Invalid expression or parse error");

	return exp;
}

ConstExpression::ConstExpression(Value _val)
	: val(_val)
{
}

Value ConstExpression::Compute(const hash_map<string, Value>& vars) const
{
	return val;
}

Expression* ConstExpression::Clone() const
{
	return new ConstExpression(*this);
}

VarExpression::VarExpression(string _name)
	: name(_name)
{
}

Value VarExpression::Compute(const hash_map<string, Value>& vars) const
{
	if (vars.find(name) == vars.end())
		throw UnknownIdentifierException(name);

	return vars.find(name)->second;
}

Expression* VarExpression::Clone() const
{
	return new VarExpression(*this);
}

void CompExpression::del()
{
	for (int i = 0; i < expressions.size(); i++)
	{
		delete expressions[i];
	}

	delete operation;
}

void CompExpression::copy(const CompExpression& obj)
{
	for (int i = 0; i < obj.expressions.size(); i++)
	{
		expressions.push_back(obj.expressions[i]->Clone());
	}

	operation = obj.operation->Clone();
}

CompExpression::CompExpression(const vector<Expression*> _expressions, IOperation* _operation)
{
	for (int i = 0; i < _expressions.size(); i++)
	{
		expressions.push_back(_expressions[i]->Clone());
	}

	operation = _operation->Clone();
}

CompExpression::CompExpression(const CompExpression& obj)
{
	copy(obj);
}

CompExpression::~CompExpression()
{
	del();
}

CompExpression& CompExpression::operator = (const CompExpression& obj)
{
	if (this != &obj)
	{
		del();
		copy(obj);
	}

	return *this;
}

Value CompExpression::Compute(const hash_map<string, Value>& vars) const
{
	vector<Value> values;

	for (int i = 0; i < expressions.size(); i++)
	{
		values.push_back(expressions[i]->Compute(vars));
	}

	return (*operation)(values);
}

Expression* CompExpression::Clone() const
{
	return new CompExpression(*this);
}