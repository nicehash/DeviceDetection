filepath = 'rawDevices.txt'
result = []

f = open("vendor_ids", "w")
with open(filepath, encoding="utf8") as fp:
    line = fp.readline()
    cnt = 1
    while line:
        if(line[0] != "#" and line[0] != "\t" and line[0] != "\n" and line[0] != "C"):
            if(not(("(Wrong ID)" in line) or ("(wrong ID)" in line))):
                f.write("0x"+line)
        line = fp.readline()
        cnt += 1
f.close()