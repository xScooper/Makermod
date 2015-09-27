#include "g_local.h"

enum MathOpTypes { VALUE, ADD, SUBTRACT, MULTIPLY, DIVIDE, BRACK_LEFT, BRACK_RIGHT };

typedef struct MathOpType{
	int type;
	char symbol;
	int order;
}MathOpType;

#define VALID_START (1 << VALUE & 1 << BRACK_LEFT)

MathOpType math_operators[] =
{
	{ VALUE,		0,	0 },
	{ ADD,			'+',1 },
	{ SUBTRACT,		'-',1 },
	{ MULTIPLY,		'*',2 },
	{ DIVIDE,		'/',2 },
	{ BRACK_LEFT,	'(',3 },
	{ BRACK_RIGHT,	')',3 },
};
int num_operators = sizeof(math_operators)/sizeof(math_operators[0]);

typedef struct MathOp{
	int type;
	int value;
	int pos;
	struct MathOp *left,*right, *back;
}MathOp;

int FormulaType(char symbol)
{
	int i;

	if(symbol >= '0' && symbol <= '9')// || symbol == '.')
		return VALUE;

	for( i = 1; i < num_operators; i++)
	{
		if(math_operators[i].symbol == symbol)
			return math_operators[i].type;
	}
}

qboolean ProcessFormula(int *values)
{
	int		i, j;
	char	buffer[MAX_TOKEN_CHARS];

	for (i = 0; i < 3 ; i++ )
	{
		int result, len;
		char *exp_p;
		int num_operators[6] = { 0 };
		MathOp root;
		MathOp *cur_pos = &root;


		trap_Argv( i+1, buffer, sizeof( buffer ) );

		// If it's just a plain number use it.
		if(isNumber(buffer,&result))
		{
			values[i] = result;
			continue;
		}

		// Otherwise check if it's a valid expression
		// And let's count supported operators first.
		//len = strlen(buffer);
		//for(j = 0; j < len; j++)
		//{
		//	if(buffer[j] == '+')
		//		num_operators[0]++;
		//	else if(buffer[j] == '-')
		//		num_operators[1]++;
		//	else if(buffer[j] == '*')
		//		num_operators[2]++;
		//	else if(buffer[j] == '/')
		//		num_operators[3]++;
		//	else if(buffer[j] == '(')
		//		num_operators[4]++;
		//	else if(buffer[j] == ')')
		//		num_operators[5]++;
		//}

		// Check if we have a valid starting point
		if(!(VALID_START & FormulaType(buffer[0])) && buffer[0] != '-')
			return qfalse;

		// Initialize the root node.
		root.pos = 0;
		root.left = root.right = root.back = NULL;
		root.value = -1;


		len = strlen(buffer);
		for(j = 0; j < len; j++)
		{
			// Check for value.
			if(FormulaType(buffer[j]) == VALUE || buffer[j] == '-')		// [FIXME]
			{
				// Read the full value.
				char full_value[256] = { 0 };
				int pos = 0;
				while((buffer[j] >= '0' && buffer[j] <= '9') || buffer[j] == '-' )
					full_value[pos++] = buffer[j++];

				cur_pos->type = VALUE;
				cur_pos->value = atoi(full_value);
			}
			// Check for parenthesis.
			else if(FormulaType(buffer[j]) == BRACK_LEFT)
			{
				cur_pos->type = BRACK_LEFT;
				if(!cur_pos->pos)
				{
					cur_pos->left = (MathOp*)malloc(sizeof(MathOp));
					cur_pos->left->back = cur_pos;
					cur_pos->pos++;
					cur_pos = cur_pos->left;
				}
				else
				{
					cur_pos->right = (MathOp*)malloc(sizeof(MathOp));
					cur_pos->right->back = cur_pos;
					cur_pos->pos++;
					cur_pos = cur_pos->right;
				}
			}
			else if(FormulaType(buffer[j]) == BRACK_RIGHT)
			{
				if(cur_pos->back->pos != 2)	// This means the expression is malformed: 5 + (5 *) for example.
					return qfalse;

				cur_pos = cur_pos->back;
			}
			else if(FormulaType(buffer[j]) == ADD)
			{
				// 5+5
				if(!cur_pos)				// Malformed: (+5
					return qfalse;

			}
		}

		exp_p = buffer;
		while(1)
		{
			// 5+5
			// 5+(5*4)
			
		}

	}
	return qtrue;
}