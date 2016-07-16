import sys


def main():

    bank_size = 2 ** 15
    bank_skip = 2

    bank_final = 32

    offset_skip = bank_skip * bank_size
    rom = open(sys.argv[1], "r").read()

    out = open(sys.argv[1].replace(".smc", ".pad"), "w")

    out.write(rom[:(bank_skip * bank_size)])

    for bank in range(bank_skip, bank_final):
        pattern = 55 + bank
        print "Pad %i Bank with %02x" % (bank, pattern)
        for i in range(0, bank_size):
            out.write(chr(pattern))
    out.close()


if __name__ == '__main__':
    main()
