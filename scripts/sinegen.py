import math

M_PI = 3.14159265358979323846


def sine(val, r, scale, stepping):
    global M_PI, flip
    re = int(math.sin(val * (M_PI * scale) / r) * r) + r
    re = re & 0xff
    re = re * stepping
    return re


def main():
    cnt = 64
    stepping = 1
    upper = 0xff

    s = "#define PWM_SINE_MAX %i\nuint8_t pwm_sine_table[] = {\n" % cnt
    for i in range(0, cnt):
        if i > 0 and i % 16 == 0:
            s += "\n"
        s += "0x%02x," % sine(i, upper / 2, (float(upper) / cnt), stepping)
    s = s[:-1]
    s += "\n};\n"
    print s

main()
