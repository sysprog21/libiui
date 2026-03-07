/*
 * Forth Salon Pixel Shader Demo
 *
 * A bytecode Forth interpreter compatible with the Forth Salon dialect
 * (forthsalon.appspot.com). Each pixel (x,y) is evaluated through a Forth
 * program that leaves R, G, B on the stack (each in 0..1).
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "forthsalon-demo.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Bytecode opcodes */

enum {
    FS_OP_HALT = 0,
    FS_OP_NUMBER,  /* followed by double */
    FS_OP_JUMP_IF, /* followed by absolute target */
    FS_OP_JUMP,    /* followed by absolute target */
    FS_OP_NOP,     /* then-marker (no-op) */

    /* Variables */
    FS_OP_X,
    FS_OP_Y,
    FS_OP_T,
    FS_OP_DT,
    FS_OP_MX,
    FS_OP_MY,

    /* Stack manipulation */
    FS_OP_DUP,
    FS_OP_2DUP,
    FS_OP_DROP,
    FS_OP_SWAP,
    FS_OP_OVER,
    FS_OP_ROT,
    FS_OP_NROT, /* -rot */

    /* Return stack */
    FS_OP_TO_R,   /* >r / push */
    FS_OP_FROM_R, /* r> / pop */
    FS_OP_R_AT,   /* r@ */

    /* Arithmetic */
    FS_OP_ADD,
    FS_OP_SUB,
    FS_OP_MUL,
    FS_OP_DIV,
    FS_OP_MOD,
    FS_OP_POW,
    FS_OP_NEGATE,

    /* Math functions */
    FS_OP_SIN,
    FS_OP_COS,
    FS_OP_TAN,
    FS_OP_ATAN2,
    FS_OP_LOG,
    FS_OP_EXP,
    FS_OP_SQRT,
    FS_OP_FLOOR,
    FS_OP_CEIL,
    FS_OP_ABS,
    FS_OP_MIN,
    FS_OP_MAX,
    FS_OP_PI,
    FS_OP_RANDOM,

    /* Comparison */
    FS_OP_EQ,
    FS_OP_NE,
    FS_OP_LT,
    FS_OP_GT,
    FS_OP_LE,
    FS_OP_GE,

    /* Logic */
    FS_OP_AND,
    FS_OP_OR,
    FS_OP_NOT,

    /* Memory */
    FS_OP_FETCH, /* @ */
    FS_OP_STORE, /* ! */

    /* Complex arithmetic */
    FS_OP_ZADD, /* z+ */
    FS_OP_ZMUL, /* z* */

    /* sf-specific: pop one value, set canvas dimension */
    FS_OP_WIDTH,
    FS_OP_HEIGHT,

    /* Stubs */
    FS_OP_SAMPLE,
    FS_OP_BWSAMPLE,
    FS_OP_AUDIO,
    FS_OP_BUTTON,
    FS_OP_BUTTONS,
};

/* Builtin word table */

typedef struct {
    const char *name;
    int opcode;
} fs_builtin_t;

static const fs_builtin_t fs_builtins[] = {
    {"x", FS_OP_X},
    {"y", FS_OP_Y},
    {"t", FS_OP_T},
    {"dt", FS_OP_DT},
    {"mx", FS_OP_MX},
    {"my", FS_OP_MY},
    {"dup", FS_OP_DUP},
    {"2dup", FS_OP_2DUP},
    {"drop", FS_OP_DROP},
    {"swap", FS_OP_SWAP},
    {"over", FS_OP_OVER},
    {"rot", FS_OP_ROT},
    {"-rot", FS_OP_NROT},
    {">r", FS_OP_TO_R},
    {"r>", FS_OP_FROM_R},
    {"r@", FS_OP_R_AT},
    {"push", FS_OP_TO_R},
    {"pop", FS_OP_FROM_R},
    {"+", FS_OP_ADD},
    {"-", FS_OP_SUB},
    {"*", FS_OP_MUL},
    {"/", FS_OP_DIV},
    {"mod", FS_OP_MOD},
    {"pow", FS_OP_POW},
    {"**", FS_OP_POW},
    {"negate", FS_OP_NEGATE},
    {"sin", FS_OP_SIN},
    {"cos", FS_OP_COS},
    {"tan", FS_OP_TAN},
    {"atan2", FS_OP_ATAN2},
    {"log", FS_OP_LOG},
    {"exp", FS_OP_EXP},
    {"sqrt", FS_OP_SQRT},
    {"floor", FS_OP_FLOOR},
    {"ceil", FS_OP_CEIL},
    {"abs", FS_OP_ABS},
    {"min", FS_OP_MIN},
    {"max", FS_OP_MAX},
    {"pi", FS_OP_PI},
    {"random", FS_OP_RANDOM},
    {"=", FS_OP_EQ},
    {"<>", FS_OP_NE},
    {"<", FS_OP_LT},
    {">", FS_OP_GT},
    {"<=", FS_OP_LE},
    {">=", FS_OP_GE},
    {"and", FS_OP_AND},
    {"or", FS_OP_OR},
    {"not", FS_OP_NOT},
    {"@", FS_OP_FETCH},
    {"!", FS_OP_STORE},
    {"z+", FS_OP_ZADD},
    {"z*", FS_OP_ZMUL},
    {"width", FS_OP_WIDTH},
    {"height", FS_OP_HEIGHT},
    {"sample", FS_OP_SAMPLE},
    {"bwsample", FS_OP_BWSAMPLE},
    {"audio", FS_OP_AUDIO},
    {"button", FS_OP_BUTTON},
    {"buttons", FS_OP_BUTTONS},
    {NULL, 0},
};

/* Parse Forth source into bytecode */

/* A user-defined word stores its compiled body in a temp buffer */
typedef struct {
    char name[FS_MAX_WORD_NAME];
    fs_inst_t body[FS_MAX_CODE / 4];
    int body_len;
} fs_word_t;

typedef struct {
    fs_program_t *prog;
    fs_word_t words[FS_MAX_WORDS];
    int n_words;

    /* Current compilation target: either main code or a word body */
    fs_inst_t *code;
    int *code_len;
    int code_cap;

    /* Jump stack for if/else/then */
    int jump_stack[32];
    int jump_top;

    int defining_word; /* -1 = compiling to main, >=0 = compiling word index */
} fs_compiler_t;

static void fs_compiler_init(fs_compiler_t *c, fs_program_t *prog)
{
    memset(c, 0, sizeof(*c));
    c->prog = prog;
    prog->code_len = 0;
    prog->canvas_w = FS_CANVAS_W;
    prog->canvas_h = FS_CANVAS_H;
    c->code = prog->code;
    c->code_len = &prog->code_len;
    c->code_cap = FS_MAX_CODE;
    c->defining_word = -1;
    c->jump_top = 0;
    c->n_words = 0;
}

static bool fs_emit(fs_compiler_t *c, fs_inst_t inst)
{
    if (*c->code_len >= c->code_cap)
        return false;
    c->code[(*c->code_len)++] = inst;
    return true;
}

static bool fs_emit_op(fs_compiler_t *c, int op)
{
    return fs_emit(c, (fs_inst_t) {.op = op});
}

static bool fs_emit_number(fs_compiler_t *c, double num)
{
    if (!fs_emit_op(c, FS_OP_NUMBER))
        return false;
    return fs_emit(c, (fs_inst_t) {.num = num});
}

static int fs_find_builtin(const char *name)
{
    for (int i = 0; fs_builtins[i].name; i++) {
        if (!strcmp(fs_builtins[i].name, name))
            return fs_builtins[i].opcode;
    }
    return -1;
}

static int fs_find_word(const fs_compiler_t *c, const char *name)
{
    for (int i = 0; i < c->n_words; i++) {
        if (!strcmp(c->words[i].name, name))
            return i;
    }
    return -1;
}

/* Inline a user-defined word's body into the current code stream */
static bool fs_inline_word(fs_compiler_t *c, int word_idx)
{
    const fs_word_t *w = &c->words[word_idx];
    int start_offset = *c->code_len;
    for (int i = 0; i < w->body_len; i++) {
        if (w->body[i].op == FS_OP_JUMP_IF || w->body[i].op == FS_OP_JUMP) {
            int op = w->body[i].op;
            if (!fs_emit_op(c, op))
                return false;
            i++;
            /* Adjust jump target relative to inlined position */
            int orig_target = w->body[i].target;
            if (!fs_emit(c, (fs_inst_t) {.target = start_offset + orig_target}))
                return false;
        } else if (w->body[i].op == FS_OP_NUMBER) {
            if (!fs_emit_op(c, FS_OP_NUMBER))
                return false;
            i++;
            if (!fs_emit(c, (fs_inst_t) {.num = w->body[i].num}))
                return false;
        } else {
            if (!fs_emit(c, w->body[i]))
                return false;
        }
    }
    return true;
}

static bool fs_parse_number(const char *word, double *out)
{
    char *end;
    *out = strtod(word, &end);
    return *end == '\0' && end != word;
}

/* Handle a single token during compilation */
static bool fs_handle_token(fs_compiler_t *c, const char *token)
{
    /* Word definition start */
    if (!strcmp(token, ":")) {
        if (c->defining_word >= 0)
            return false; /* nested definitions not allowed */
        /* The next token will be the word name — signal by setting
         * defining_word to a sentinel
         */
        c->defining_word = -2; /* waiting for name */
        return true;
    }

    /* If we're waiting for a word name after ':' */
    if (c->defining_word == -2) {
        if (c->n_words >= FS_MAX_WORDS)
            return false;
        int idx = c->n_words++;
        size_t len = strlen(token);
        if (len >= FS_MAX_WORD_NAME)
            return false;
        memcpy(c->words[idx].name, token, len + 1);
        c->words[idx].body_len = 0;
        c->defining_word = idx;
        /* Redirect compilation to word body */
        c->code = c->words[idx].body;
        c->code_len = &c->words[idx].body_len;
        c->code_cap = (int) (sizeof(c->words[idx].body) / sizeof(fs_inst_t));
        c->jump_top = 0;
        return true;
    }

    /* Word definition end */
    if (!strcmp(token, ";")) {
        if (c->defining_word < 0)
            return false;
        /* Switch back to main code */
        c->defining_word = -1;
        c->code = c->prog->code;
        c->code_len = &c->prog->code_len;
        c->code_cap = FS_MAX_CODE;
        c->jump_top = 0;
        return true;
    }

    /* Comment: backslash to end of line — handled in tokenizer
     * Comment: ( ... ) — handled in tokenizer
     */

    /* Control flow: if */
    if (!strcmp(token, "if")) {
        if (c->jump_top >= 32)
            return false;
        if (!fs_emit_op(c, FS_OP_JUMP_IF))
            return false;
        c->jump_stack[c->jump_top++] = *c->code_len;
        return fs_emit(c, (fs_inst_t) {.target = 0}); /* placeholder */
    }

    /* Control flow: else */
    if (!strcmp(token, "else")) {
        if (c->jump_top < 1)
            return false;
        int if_target_slot = c->jump_stack[--c->jump_top];
        if (!fs_emit_op(c, FS_OP_JUMP))
            return false;
        c->jump_stack[c->jump_top++] = *c->code_len;
        if (!fs_emit(c, (fs_inst_t) {.target = 0}))
            return false;
        /* Patch the if-branch to jump here */
        c->code[if_target_slot].target = *c->code_len;
        return true;
    }

    /* Control flow: then */
    if (!strcmp(token, "then")) {
        if (c->jump_top < 1)
            return false;
        int target_slot = c->jump_stack[--c->jump_top];
        c->code[target_slot].target = *c->code_len;
        return true;
    }

    /* Try as number */
    double num;
    if (fs_parse_number(token, &num))
        return fs_emit_number(c, num);

    /* Try as builtin */
    int op = fs_find_builtin(token);
    if (op >= 0)
        return fs_emit_op(c, op);

    /* Try as user-defined word (inline it) */
    int wi = fs_find_word(c, token);
    if (wi >= 0)
        return fs_inline_word(c, wi);

    return false; /* unknown word */
}

static bool fs_compile(fs_program_t *prog, const char *source)
{
    fs_compiler_t compiler;
    fs_compiler_init(&compiler, prog);

    const char *p = source;
    char token[FS_MAX_WORD_NAME];

    while (*p) {
        /* Skip whitespace */
        while (*p && isspace((unsigned char) *p))
            p++;
        if (!*p)
            break;

        /* Handle line comments: \ to end of line */
        if (*p == '\\') {
            while (*p && *p != '\n')
                p++;
            continue;
        }

        /* Handle parenthesized comments: ( ... ) */
        if (*p == '(') {
            while (*p && *p != ')')
                p++;
            if (*p == ')')
                p++;
            continue;
        }

        /* Extract token */
        int len = 0;
        while (*p && !isspace((unsigned char) *p) &&
               len < FS_MAX_WORD_NAME - 1) {
            token[len++] = *p++;
        }
        token[len] = '\0';

        if (!fs_handle_token(&compiler, token))
            return false;
    }

    /* Emit halt */
    if (!fs_emit_op(&compiler, FS_OP_HALT))
        return false;

    return true;
}

/* VM execution */

typedef struct {
    double x, y, t, dt;
    double memory[FS_MEMORY_SLOTS];
} fs_vars_t;

static void fs_run(const fs_program_t *prog,
                   fs_vars_t *vars,
                   double *d_stack,
                   int *d_top)
{
    double r_stack[FS_R_STACK_SIZE];
    int r_top = 0;
    int pc = 0;
    int ds = 0;

#define DPUSH(v)                  \
    do {                          \
        if (ds < FS_D_STACK_SIZE) \
            d_stack[ds++] = (v);  \
    } while (0)
#define DPOP() (ds > 0 ? d_stack[--ds] : 0.0)
#define RPUSH(v)                     \
    do {                             \
        if (r_top < FS_R_STACK_SIZE) \
            r_stack[r_top++] = (v);  \
    } while (0)
#define RPOP() (r_top > 0 ? r_stack[--r_top] : 0.0)

    while (pc < prog->code_len) {
        int op = prog->code[pc].op;
        pc++;

        switch (op) {
        case FS_OP_HALT:
            goto done;

        case FS_OP_NUMBER:
            DPUSH(prog->code[pc].num);
            pc++;
            break;

        case FS_OP_JUMP_IF: {
            int target = prog->code[pc].target;
            pc++;
            if (DPOP() == 0.0)
                pc = target;
            break;
        }

        case FS_OP_JUMP:
            pc = prog->code[pc].target;
            break;

        case FS_OP_NOP:
            break;

        case FS_OP_X:
            DPUSH(vars->x);
            break;
        case FS_OP_Y:
            DPUSH(vars->y);
            break;
        case FS_OP_T:
            DPUSH(vars->t);
            break;
        case FS_OP_DT:
            DPUSH(vars->dt);
            break;
        case FS_OP_MX:
            DPUSH(0.0);
            break;
        case FS_OP_MY:
            DPUSH(0.0);
            break;

        case FS_OP_DUP: {
            double v = ds > 0 ? d_stack[ds - 1] : 0.0;
            DPUSH(v);
            break;
        }
        case FS_OP_2DUP: {
            double b = ds > 0 ? d_stack[ds - 1] : 0.0;
            double a = ds > 1 ? d_stack[ds - 2] : 0.0;
            DPUSH(a);
            DPUSH(b);
            break;
        }
        case FS_OP_DROP:
            if (ds > 0)
                ds--;
            break;
        case FS_OP_SWAP: {
            if (ds >= 2) {
                double a = d_stack[ds - 2];
                d_stack[ds - 2] = d_stack[ds - 1];
                d_stack[ds - 1] = a;
            }
            break;
        }
        case FS_OP_OVER: {
            double v = ds >= 2 ? d_stack[ds - 2] : 0.0;
            DPUSH(v);
            break;
        }
        case FS_OP_ROT: {
            double c = DPOP();
            double b = DPOP();
            double a = DPOP();
            DPUSH(b);
            DPUSH(c);
            DPUSH(a);
            break;
        }
        case FS_OP_NROT: {
            double c = DPOP();
            double b = DPOP();
            double a = DPOP();
            DPUSH(c);
            DPUSH(a);
            DPUSH(b);
            break;
        }

        case FS_OP_TO_R:
            RPUSH(DPOP());
            break;
        case FS_OP_FROM_R:
            DPUSH(RPOP());
            break;
        case FS_OP_R_AT:
            DPUSH(r_top > 0 ? r_stack[r_top - 1] : 0.0);
            break;

        case FS_OP_ADD: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a + b);
            break;
        }
        case FS_OP_SUB: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a - b);
            break;
        }
        case FS_OP_MUL: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a * b);
            break;
        }
        case FS_OP_DIV: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(b == 0.0 ? INFINITY : a / b);
            break;
        }
        case FS_OP_MOD: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(fmod(a, b));
            break;
        }
        case FS_OP_POW: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(pow(fabs(a), b));
            break;
        }
        case FS_OP_NEGATE:
            if (ds > 0)
                d_stack[ds - 1] = -d_stack[ds - 1];
            break;

        case FS_OP_SIN:
            if (ds > 0)
                d_stack[ds - 1] = sin(d_stack[ds - 1]);
            break;
        case FS_OP_COS:
            if (ds > 0)
                d_stack[ds - 1] = cos(d_stack[ds - 1]);
            break;
        case FS_OP_TAN:
            if (ds > 0)
                d_stack[ds - 1] = tan(d_stack[ds - 1]);
            break;
        case FS_OP_ATAN2: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(atan2(a, b));
            break;
        }
        case FS_OP_LOG:
            if (ds > 0)
                d_stack[ds - 1] = log(fabs(d_stack[ds - 1]));
            break;
        case FS_OP_EXP:
            if (ds > 0)
                d_stack[ds - 1] = exp(d_stack[ds - 1]);
            break;
        case FS_OP_SQRT:
            if (ds > 0)
                d_stack[ds - 1] = sqrt(fabs(d_stack[ds - 1]));
            break;
        case FS_OP_FLOOR:
            if (ds > 0)
                d_stack[ds - 1] = floor(d_stack[ds - 1]);
            break;
        case FS_OP_CEIL:
            if (ds > 0)
                d_stack[ds - 1] = ceil(d_stack[ds - 1]);
            break;
        case FS_OP_ABS:
            if (ds > 0)
                d_stack[ds - 1] = fabs(d_stack[ds - 1]);
            break;
        case FS_OP_MIN: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(fmin(a, b));
            break;
        }
        case FS_OP_MAX: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(fmax(a, b));
            break;
        }
        case FS_OP_PI:
            DPUSH(M_PI);
            break;
        case FS_OP_RANDOM:
            DPUSH((double) rand() / (double) RAND_MAX);
            break;

        case FS_OP_EQ: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a == b ? 1.0 : 0.0);
            break;
        }
        case FS_OP_NE: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a != b ? 1.0 : 0.0);
            break;
        }
        case FS_OP_LT: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a < b ? 1.0 : 0.0);
            break;
        }
        case FS_OP_GT: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a > b ? 1.0 : 0.0);
            break;
        }
        case FS_OP_LE: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a <= b ? 1.0 : 0.0);
            break;
        }
        case FS_OP_GE: {
            double b = DPOP();
            double a = DPOP();
            DPUSH(a >= b ? 1.0 : 0.0);
            break;
        }

        case FS_OP_AND: {
            double b = DPOP();
            double a = DPOP();
            DPUSH((a != 0.0 && b != 0.0) ? 1.0 : 0.0);
            break;
        }
        case FS_OP_OR: {
            double b = DPOP();
            double a = DPOP();
            DPUSH((a != 0.0 || b != 0.0) ? 1.0 : 0.0);
            break;
        }
        case FS_OP_NOT:
            if (ds > 0)
                d_stack[ds - 1] = (d_stack[ds - 1] != 0.0) ? 0.0 : 1.0;
            break;

        case FS_OP_FETCH: {
            double sval = DPOP();
            int slot = isfinite(sval) ? (int) sval : 0;
            slot =
                ((slot % FS_MEMORY_SLOTS) + FS_MEMORY_SLOTS) % FS_MEMORY_SLOTS;
            DPUSH(vars->memory[slot]);
            break;
        }
        case FS_OP_STORE: {
            double v = DPOP();
            double sval = DPOP();
            int slot = isfinite(sval) ? (int) sval : 0;
            slot =
                ((slot % FS_MEMORY_SLOTS) + FS_MEMORY_SLOTS) % FS_MEMORY_SLOTS;
            vars->memory[slot] = v;
            break;
        }

        case FS_OP_ZADD: {
            double bi = DPOP();
            double br = DPOP();
            double ai = DPOP();
            double ar = DPOP();
            DPUSH(ar + br);
            DPUSH(ai + bi);
            break;
        }
        case FS_OP_ZMUL: {
            double bi = DPOP();
            double br = DPOP();
            double ai = DPOP();
            double ar = DPOP();
            DPUSH(ar * br - ai * bi);
            DPUSH(ar * bi + ai * br);
            break;
        }

        case FS_OP_WIDTH:
            /* pop canvas width (ignored — we use fixed size) */
            if (ds > 0)
                ds--;
            break;
        case FS_OP_HEIGHT:
            if (ds > 0)
                ds--;
            break;

        case FS_OP_SAMPLE:
            /* stub: pop x,y, push r,g,b=0 */
            if (ds >= 2)
                ds -= 2;
            else
                ds = 0;
            DPUSH(0);
            DPUSH(0);
            DPUSH(0);
            break;
        case FS_OP_BWSAMPLE:
            if (ds >= 2)
                ds -= 2;
            else
                ds = 0;
            DPUSH(0);
            break;
        case FS_OP_AUDIO:
            if (ds > 0)
                ds--;
            break;
        case FS_OP_BUTTON:
            if (ds > 0)
                d_stack[ds - 1] = 0.0;
            break;
        case FS_OP_BUTTONS:
            DPUSH(0.0);
            break;
        }
    }

done:
    *d_top = ds;

#undef DPUSH
#undef DPOP
#undef RPUSH
#undef RPOP
}

/* Built-in examples from https://github.com/boomlinde/sf */

typedef struct {
    const char *name;
    const char *source;
} fs_example_t;

static const fs_example_t fs_examples[] = {
    {
        "Twister",
        ": t t pi * 2 / ;\n"
        ": l * + sin ;\n"
        ": r t 1 y t + 4 l + 1.57 ;\n"
        ": x x 4 * 2 - t y 3 l + ;\n"
        ": v 2dup x >= swap x < * -rot swap - l ;\n"
        ": a r 4 l ; : b r 1 l ;\n"
        ": c r 2 l ; : d r 3 l ;\n"
        "0 d a v a b v b c v c d v 0.1 0.2",
    },
    {
        "Rainbow",
        ": ' .5 - 2 * ;\n"
        ": d x ' dup * y ' dup * + sqrt ;\n"
        ": p 2 * pi * cos .5 * .5 + ;\n"
        ": col dup dup p swap 1 3 / + p rot 2 3 / + p ;\n"
        "y ' x ' atan2 pi 2 * / t + d t 2 pi * * sin * + col",
    },
    {
        "Checker",
        ": n .5 - 2 * ;\n"
        ": z y n 2 * / 1 mod ;\n"
        ": m 2 * 2 mod floor ;\n"
        ": c t + m swap m + 2 mod 0 = ;\n"
        "x n z 2 z c y n abs *",
    },
    {
        "Metadonut",
        ": xp t pi * 4 / sin .3 * ;\n"
        ": yp t pi * cos .3 * ;\n"
        ": d dup * swap dup * + sqrt ;\n"
        ": p y .5 - + swap x .5 - + d 1 swap - 5 pow ;\n"
        ": col dup dup .2 > swap .4 < and swap .2 - 5 * * ;\n"
        "xp yp p xp negate yp negate p + col\n"
        "dup 4 pow dup rot swap .1 +",
    },
};

#define FS_EXAMPLE_COUNT (int) (sizeof(fs_examples) / sizeof(fs_examples[0]))

/* Pixel rendering */

static uint8_t fs_clamp_byte(double v)
{
    if (!(v >= 0.0))
        return 0;
    if (v >= 1.0)
        return 255;
    return (uint8_t) (v * 255.0 + 0.5);
}

/* Evaluate the program at a single (x,y,t) point and extract RGB.
 * Returns accumulated (r,g,b) in the out[] array (values in 0..1).
 */
static void fs_eval_pixel(const fs_program_t *prog,
                          double px,
                          double py,
                          double t,
                          double dt,
                          double out[3])
{
    double d_stack[FS_D_STACK_SIZE];
    int d_top = 0;
    fs_vars_t vars = {.x = px, .y = py, .t = t, .dt = dt};

    fs_run(prog, &vars, d_stack, &d_top);

    if (d_top >= 3) {
        out[0] = d_stack[d_top - 3];
        out[1] = d_stack[d_top - 2];
        out[2] = d_stack[d_top - 1];
    } else if (d_top >= 1) {
        out[0] = out[1] = out[2] = d_stack[d_top - 1];
    } else {
        out[0] = out[1] = out[2] = 0.0;
    }
}

static void fs_render_frame(const fs_program_t *prog,
                            uint32_t canvas[][FS_CANVAS_W],
                            double t,
                            double dt)
{
    int w = prog->canvas_w;
    int h = prog->canvas_h;
    if (w > FS_CANVAS_W)
        w = FS_CANVAS_W;
    if (h > FS_CANVAS_H)
        h = FS_CANVAS_H;

    double inv_w = 1.0 / (double) w;
    double inv_h = 1.0 / (double) h;

    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            double rgb[3];
            double sx = ((double) px + 0.5) * inv_w;
            double sy = ((double) py + 0.5) * inv_h;
            fs_eval_pixel(prog, sx, sy, t, dt, rgb);

            uint8_t r = fs_clamp_byte(rgb[0]);
            uint8_t g = fs_clamp_byte(rgb[1]);
            uint8_t b = fs_clamp_byte(rgb[2]);

            canvas[py][px] =
                (0xFFu << 24) | ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
        }
    }
}

/* Canvas rendering via libIUI draw_box */

static void fs_render_canvas(iui_port_ctx *port,
                             const uint32_t canvas[][FS_CANVAS_W],
                             int w,
                             int h,
                             iui_rect_t area)
{
    iui_renderer_t cb = g_iui_port.get_renderer_callbacks(port);

    /* Preserve square aspect ratio, centered in the available area */
    float side = (area.width < area.height) ? area.width : area.height;
    float ox = area.x + (area.width - side) * 0.5f;
    float oy = area.y + (area.height - side) * 0.5f;
    iui_rect_t sq = {ox, oy, side, side};

    /* Black background */
    cb.draw_box(sq, 0, 0xFF000000, cb.user);

    float cw = side / (float) w;
    float ch = side / (float) h;

    /* Row-based RLE with pixel-boundary snapping.
     * floorf/ceilf on cell edges ensures adjacent cells share the exact same
     * boundary pixel, eliminating sub-pixel gaps between rows.
     */
    for (int py = 0; py < h; py++) {
        float y0 = floorf(oy + (float) py * ch);
        float y1 = ceilf(oy + (float) (py + 1) * ch);
        int px = 0;
        while (px < w) {
            uint32_t argb = canvas[py][px];
            int run = 1;
            while (px + run < w && canvas[py][px + run] == argb)
                run++;
            if ((argb & 0x00FFFFFF) != 0) {
                float x0 = floorf(ox + (float) px * cw);
                float x1 = ceilf(ox + (float) (px + run) * cw);
                iui_rect_t r = {x0, y0, x1 - x0, y1 - y0};
                cb.draw_box(r, 0, argb, cb.user);
            }
            px += run;
        }
    }
}

static void fs_switch_example(forthsalon_state_t *fs, int idx)
{
    memset(&fs->program, 0, sizeof(fs->program));
    memset(fs->canvas, 0, sizeof(fs->canvas));
    fs->compiled = fs_compile(&fs->program, fs_examples[idx].source);
    fs->time = 0.0;
}

void forthsalon_init(forthsalon_state_t *fs)
{
    memset(fs, 0, sizeof(*fs));
    fs->tab = 0;
    fs_switch_example(fs, 0);
}

void draw_forthsalon_window(iui_context *ui,
                            iui_port_ctx *port,
                            forthsalon_state_t *fs,
                            float dt,
                            float win_h)
{
    if (!iui_begin_window(ui, "Forth Salon", 380, 30, 420, win_h,
                          IUI_WINDOW_RESIZABLE))
        return;

    /* Example selector tabs */
    static const char *labels[4];
    static bool labels_init = false;
    if (!labels_init) {
        for (int i = 0; i < FS_EXAMPLE_COUNT; i++)
            labels[i] = fs_examples[i].name;
        labels_init = true;
    }

    int new_tab = iui_tabs(ui, fs->tab, FS_EXAMPLE_COUNT, labels);
    if (new_tab != fs->tab) {
        fs_switch_example(fs, new_tab);
        fs->tab = new_tab;
    }

    /* Advance time and render the pixel shader */
    if (fs->compiled) {
        fs->time += (double) dt;
        fs_render_frame(&fs->program, fs->canvas, fs->time, (double) dt);
    }

    /* Canvas render area */
    float avail_h = iui_get_remaining_height(ui);
    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = avail_h});
    iui_rect_t area = iui_box_next(ui);

    int cw = fs->program.canvas_w;
    int ch = fs->program.canvas_h;
    if (cw > FS_CANVAS_W)
        cw = FS_CANVAS_W;
    if (ch > FS_CANVAS_H)
        ch = FS_CANVAS_H;

    fs_render_canvas(port, fs->canvas, cw, ch, area);
    iui_box_end(ui);

    iui_end_window(ui);
}
