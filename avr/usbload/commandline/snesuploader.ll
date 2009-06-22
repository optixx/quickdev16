; ModuleID = '/Users/david/Devel/arch/avr/code/snesram/poc/avr_usbload/commandline/snesuploader.c'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128"
target triple = "i386-apple-darwin9"
	%struct.__sFILE = type <{ i8*, i32, i32, i16, i16, %struct.__sbuf, i32, i8*, i32 (i8*)*, i32 (i8*, i8*, i32)*, i64 (i8*, i64, i32)*, i32 (i8*, i8*, i32)*, %struct.__sbuf, %struct.__sFILEX*, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64 }>
	%struct.__sFILEX = type opaque
	%struct.__sbuf = type <{ i8*, i32 }>
	%struct.usb_dev_handle = type opaque
@"\01LC" = internal constant [3 x i8] c"*\0A\00"		; <[3 x i8]*> [#uses=1]
@"\01LC1" = internal constant [6 x i8] c"%08x:\00"		; <[6 x i8]*> [#uses=1]
@"\01LC2" = internal constant [6 x i8] c" %02x\00"		; <[6 x i8]*> [#uses=1]
@"\01LC3" = internal constant [3 x i8] c" |\00"		; <[3 x i8]*> [#uses=1]
@"\01LC4" = internal constant [3 x i8] c"%c\00"		; <[3 x i8]*> [#uses=1]
@"\01LC5" = internal constant [2 x i8] c".\00"		; <[2 x i8]*> [#uses=1]
@"\01LC6" = internal constant [3 x i8] c"|\0A\00"		; <[3 x i8]*> [#uses=1]
@__stderrp = external global %struct.__sFILE*		; <%struct.__sFILE**> [#uses=8]
@"\01LC7" = internal constant [55 x i8] c"Could not find USB device \22%s\22 with vid=0x%x pid=0x%x\0A\00"		; <[55 x i8]*> [#uses=1]
@"\01LC8" = internal constant [45 x i8] c"Open USB device \22%s\22 with vid=0x%x pid=0x%x\0A\00"		; <[45 x i8]*> [#uses=1]
@"\01LC9" = internal constant [7 x i8] c"upload\00"		; <[7 x i8]*> [#uses=1]
@"\01LC10" = internal constant [2 x i8] c"r\00"		; <[2 x i8]*> [#uses=1]
@"\01LC11" = internal constant [21 x i8] c"Cannot open file %s \00"		; <[21 x i8]*> [#uses=1]
@"\01LC12" = internal constant [70 x i8] c"Addr: 0x%06x Bank: 0x%02x HiAddr: 0x%02x  LoAddr: 0x%04x Crc: 0x%04x\0A\00"		; <[70 x i8]*> [#uses=1]
@"\01LC13" = internal constant [15 x i8] c"USB error: %s\0A\00"		; <[15 x i8]*> [#uses=1]
@"\01LC14" = internal constant [25 x i8] c"only %d bytes received.\0A\00"		; <[25 x i8]*> [#uses=1]
@"\01LC15" = internal constant [4 x i8] c"crc\00"		; <[4 x i8]*> [#uses=1]
@"\01LC16" = internal constant [30 x i8] c"Request CRC for Addr: 0x%06x\0A\00"		; <[30 x i8]*> [#uses=1]
@"\01LC17" = internal constant [8 x i8] c"usage:\0A\00"		; <[8 x i8]*> [#uses=1]
@"\01LC18" = internal constant [31 x i8] c"  %s upload filename.. upload\0A\00"		; <[31 x i8]*> [#uses=1]

define void @dump_packet(i32 %addr, i32 %len, i8* %packet) nounwind {
entry:
	%addr.addr = alloca i32		; <i32*> [#uses=2]
	%len.addr = alloca i32		; <i32*> [#uses=2]
	%packet.addr = alloca i8*		; <i8**> [#uses=6]
	%i = alloca i16, align 2		; <i16*> [#uses=10]
	%j = alloca i16, align 2		; <i16*> [#uses=17]
	%sum = alloca i16, align 2		; <i16*> [#uses=5]
	%clear = alloca i8, align 1		; <i8*> [#uses=4]
	store i32 %addr, i32* %addr.addr
	store i32 %len, i32* %len.addr
	store i8* %packet, i8** %packet.addr
	store i16 0, i16* %sum
	store i8 0, i8* %clear
	store i16 0, i16* %i
	br label %for.cond

for.cond:		; preds = %for.inc98, %entry
	%tmp = load i16* %i		; <i16> [#uses=1]
	%conv = zext i16 %tmp to i32		; <i32> [#uses=1]
	%tmp1 = load i32* %len.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %conv, %tmp1		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end103

for.body:		; preds = %for.cond
	store i16 0, i16* %sum
	store i16 0, i16* %j
	br label %for.cond3

for.cond3:		; preds = %for.inc, %for.body
	%tmp4 = load i16* %j		; <i16> [#uses=1]
	%conv5 = zext i16 %tmp4 to i32		; <i32> [#uses=1]
	%cmp6 = icmp slt i32 %conv5, 16		; <i1> [#uses=1]
	br i1 %cmp6, label %for.body8, label %for.end

for.body8:		; preds = %for.cond3
	%tmp9 = load i16* %sum		; <i16> [#uses=1]
	%conv10 = zext i16 %tmp9 to i32		; <i32> [#uses=1]
	%tmp11 = load i16* %i		; <i16> [#uses=1]
	%conv12 = zext i16 %tmp11 to i32		; <i32> [#uses=1]
	%tmp13 = load i16* %j		; <i16> [#uses=1]
	%conv14 = zext i16 %tmp13 to i32		; <i32> [#uses=1]
	%add = add i32 %conv12, %conv14		; <i32> [#uses=1]
	%tmp15 = load i8** %packet.addr		; <i8*> [#uses=1]
	%arrayidx = getelementptr i8* %tmp15, i32 %add		; <i8*> [#uses=1]
	%tmp16 = load i8* %arrayidx		; <i8> [#uses=1]
	%conv17 = zext i8 %tmp16 to i32		; <i32> [#uses=1]
	%add18 = add i32 %conv10, %conv17		; <i32> [#uses=1]
	%conv19 = trunc i32 %add18 to i16		; <i16> [#uses=1]
	store i16 %conv19, i16* %sum
	br label %for.inc

for.inc:		; preds = %for.body8
	%tmp20 = load i16* %j		; <i16> [#uses=1]
	%inc = add i16 %tmp20, 1		; <i16> [#uses=1]
	store i16 %inc, i16* %j
	br label %for.cond3

for.end:		; preds = %for.cond3
	%tmp21 = load i16* %sum		; <i16> [#uses=1]
	%tobool = icmp ne i16 %tmp21, 0		; <i1> [#uses=1]
	br i1 %tobool, label %if.end, label %if.then

if.then:		; preds = %for.end
	store i8 1, i8* %clear
	br label %for.inc98

if.end:		; preds = %for.end
	%tmp22 = load i8* %clear		; <i8> [#uses=1]
	%tobool23 = icmp ne i8 %tmp22, 0		; <i1> [#uses=1]
	br i1 %tobool23, label %if.then24, label %if.end25

if.then24:		; preds = %if.end
	%call = call i32 (i8*, ...)* @printf(i8* getelementptr ([3 x i8]* @"\01LC", i32 0, i32 0))		; <i32> [#uses=0]
	store i8 0, i8* %clear
	br label %if.end25

if.end25:		; preds = %if.then24, %if.end
	%tmp26 = load i32* %addr.addr		; <i32> [#uses=1]
	%tmp27 = load i16* %i		; <i16> [#uses=1]
	%conv28 = zext i16 %tmp27 to i32		; <i32> [#uses=1]
	%add29 = add i32 %tmp26, %conv28		; <i32> [#uses=1]
	%call30 = call i32 (i8*, ...)* @printf(i8* getelementptr ([6 x i8]* @"\01LC1", i32 0, i32 0), i32 %add29)		; <i32> [#uses=0]
	store i16 0, i16* %j
	br label %for.cond31

for.cond31:		; preds = %for.inc47, %if.end25
	%tmp32 = load i16* %j		; <i16> [#uses=1]
	%conv33 = zext i16 %tmp32 to i32		; <i32> [#uses=1]
	%cmp34 = icmp slt i32 %conv33, 16		; <i1> [#uses=1]
	br i1 %cmp34, label %for.body36, label %for.end50

for.body36:		; preds = %for.cond31
	%tmp37 = load i16* %i		; <i16> [#uses=1]
	%conv38 = zext i16 %tmp37 to i32		; <i32> [#uses=1]
	%tmp39 = load i16* %j		; <i16> [#uses=1]
	%conv40 = zext i16 %tmp39 to i32		; <i32> [#uses=1]
	%add41 = add i32 %conv38, %conv40		; <i32> [#uses=1]
	%tmp42 = load i8** %packet.addr		; <i8*> [#uses=1]
	%arrayidx43 = getelementptr i8* %tmp42, i32 %add41		; <i8*> [#uses=1]
	%tmp44 = load i8* %arrayidx43		; <i8> [#uses=1]
	%conv45 = zext i8 %tmp44 to i32		; <i32> [#uses=1]
	%call46 = call i32 (i8*, ...)* @printf(i8* getelementptr ([6 x i8]* @"\01LC2", i32 0, i32 0), i32 %conv45)		; <i32> [#uses=0]
	br label %for.inc47

for.inc47:		; preds = %for.body36
	%tmp48 = load i16* %j		; <i16> [#uses=1]
	%inc49 = add i16 %tmp48, 1		; <i16> [#uses=1]
	store i16 %inc49, i16* %j
	br label %for.cond31

for.end50:		; preds = %for.cond31
	%call51 = call i32 (i8*, ...)* @printf(i8* getelementptr ([3 x i8]* @"\01LC3", i32 0, i32 0))		; <i32> [#uses=0]
	store i16 0, i16* %j
	br label %for.cond52

for.cond52:		; preds = %for.inc93, %for.end50
	%tmp53 = load i16* %j		; <i16> [#uses=1]
	%conv54 = zext i16 %tmp53 to i32		; <i32> [#uses=1]
	%cmp55 = icmp slt i32 %conv54, 16		; <i1> [#uses=1]
	br i1 %cmp55, label %for.body57, label %for.end96

for.body57:		; preds = %for.cond52
	%tmp58 = load i16* %i		; <i16> [#uses=1]
	%conv59 = zext i16 %tmp58 to i32		; <i32> [#uses=1]
	%tmp60 = load i16* %j		; <i16> [#uses=1]
	%conv61 = zext i16 %tmp60 to i32		; <i32> [#uses=1]
	%add62 = add i32 %conv59, %conv61		; <i32> [#uses=1]
	%tmp63 = load i8** %packet.addr		; <i8*> [#uses=1]
	%arrayidx64 = getelementptr i8* %tmp63, i32 %add62		; <i8*> [#uses=1]
	%tmp65 = load i8* %arrayidx64		; <i8> [#uses=1]
	%conv66 = zext i8 %tmp65 to i32		; <i32> [#uses=1]
	%cmp67 = icmp sge i32 %conv66, 33		; <i1> [#uses=1]
	br i1 %cmp67, label %land.lhs.true, label %if.else

land.lhs.true:		; preds = %for.body57
	%tmp69 = load i16* %i		; <i16> [#uses=1]
	%conv70 = zext i16 %tmp69 to i32		; <i32> [#uses=1]
	%tmp71 = load i16* %j		; <i16> [#uses=1]
	%conv72 = zext i16 %tmp71 to i32		; <i32> [#uses=1]
	%add73 = add i32 %conv70, %conv72		; <i32> [#uses=1]
	%tmp74 = load i8** %packet.addr		; <i8*> [#uses=1]
	%arrayidx75 = getelementptr i8* %tmp74, i32 %add73		; <i8*> [#uses=1]
	%tmp76 = load i8* %arrayidx75		; <i8> [#uses=1]
	%conv77 = zext i8 %tmp76 to i32		; <i32> [#uses=1]
	%cmp78 = icmp sle i32 %conv77, 126		; <i1> [#uses=1]
	br i1 %cmp78, label %if.then80, label %if.else

if.then80:		; preds = %land.lhs.true
	%tmp81 = load i16* %i		; <i16> [#uses=1]
	%conv82 = zext i16 %tmp81 to i32		; <i32> [#uses=1]
	%tmp83 = load i16* %j		; <i16> [#uses=1]
	%conv84 = zext i16 %tmp83 to i32		; <i32> [#uses=1]
	%add85 = add i32 %conv82, %conv84		; <i32> [#uses=1]
	%tmp86 = load i8** %packet.addr		; <i8*> [#uses=1]
	%arrayidx87 = getelementptr i8* %tmp86, i32 %add85		; <i8*> [#uses=1]
	%tmp88 = load i8* %arrayidx87		; <i8> [#uses=1]
	%conv89 = zext i8 %tmp88 to i32		; <i32> [#uses=1]
	%call90 = call i32 (i8*, ...)* @printf(i8* getelementptr ([3 x i8]* @"\01LC4", i32 0, i32 0), i32 %conv89)		; <i32> [#uses=0]
	br label %if.end92

if.else:		; preds = %land.lhs.true, %for.body57
	%call91 = call i32 (i8*, ...)* @printf(i8* getelementptr ([2 x i8]* @"\01LC5", i32 0, i32 0))		; <i32> [#uses=0]
	br label %if.end92

if.end92:		; preds = %if.else, %if.then80
	br label %for.inc93

for.inc93:		; preds = %if.end92
	%tmp94 = load i16* %j		; <i16> [#uses=1]
	%inc95 = add i16 %tmp94, 1		; <i16> [#uses=1]
	store i16 %inc95, i16* %j
	br label %for.cond52

for.end96:		; preds = %for.cond52
	%call97 = call i32 (i8*, ...)* @printf(i8* getelementptr ([3 x i8]* @"\01LC6", i32 0, i32 0))		; <i32> [#uses=0]
	br label %for.inc98

for.inc98:		; preds = %for.end96, %if.then
	%tmp99 = load i16* %i		; <i16> [#uses=1]
	%conv100 = zext i16 %tmp99 to i32		; <i32> [#uses=1]
	%add101 = add i32 %conv100, 16		; <i32> [#uses=1]
	%conv102 = trunc i32 %add101 to i16		; <i16> [#uses=1]
	store i16 %conv102, i16* %i
	br label %for.cond

for.end103:		; preds = %for.cond
	ret void
}

declare i32 @printf(i8*, ...)

define zeroext i16 @crc_xmodem_update(i16 zeroext %crc, i8 zeroext %data) nounwind {
entry:
	%retval = alloca i16		; <i16*> [#uses=2]
	%crc.addr = alloca i16		; <i16*> [#uses=9]
	%data.addr = alloca i8		; <i8*> [#uses=2]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	store i16 %crc, i16* %crc.addr
	store i8 %data, i8* %data.addr
	%tmp = load i16* %crc.addr		; <i16> [#uses=1]
	%conv = zext i16 %tmp to i32		; <i32> [#uses=1]
	%tmp1 = load i8* %data.addr		; <i8> [#uses=1]
	%conv2 = zext i8 %tmp1 to i32		; <i32> [#uses=1]
	%conv3 = trunc i32 %conv2 to i16		; <i16> [#uses=1]
	%conv4 = zext i16 %conv3 to i32		; <i32> [#uses=1]
	%shl = shl i32 %conv4, 8		; <i32> [#uses=1]
	%xor = xor i32 %conv, %shl		; <i32> [#uses=1]
	%conv5 = trunc i32 %xor to i16		; <i16> [#uses=1]
	store i16 %conv5, i16* %crc.addr
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp6 = load i32* %i		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp6, 8		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp8 = load i16* %crc.addr		; <i16> [#uses=1]
	%conv9 = zext i16 %tmp8 to i32		; <i32> [#uses=1]
	%and = and i32 %conv9, 32768		; <i32> [#uses=1]
	%tobool = icmp ne i32 %and, 0		; <i1> [#uses=1]
	br i1 %tobool, label %if.then, label %if.else

if.then:		; preds = %for.body
	%tmp10 = load i16* %crc.addr		; <i16> [#uses=1]
	%conv11 = zext i16 %tmp10 to i32		; <i32> [#uses=1]
	%shl12 = shl i32 %conv11, 1		; <i32> [#uses=1]
	%xor13 = xor i32 %shl12, 4129		; <i32> [#uses=1]
	%conv14 = trunc i32 %xor13 to i16		; <i16> [#uses=1]
	store i16 %conv14, i16* %crc.addr
	br label %if.end

if.else:		; preds = %for.body
	%tmp15 = load i16* %crc.addr		; <i16> [#uses=1]
	%conv16 = zext i16 %tmp15 to i32		; <i32> [#uses=1]
	%shl17 = shl i32 %conv16, 1		; <i32> [#uses=1]
	%conv18 = trunc i32 %shl17 to i16		; <i16> [#uses=1]
	store i16 %conv18, i16* %crc.addr
	br label %if.end

if.end:		; preds = %if.else, %if.then
	br label %for.inc

for.inc:		; preds = %if.end
	%tmp19 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp19, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp20 = load i16* %crc.addr		; <i16> [#uses=1]
	store i16 %tmp20, i16* %retval
	%0 = load i16* %retval		; <i16> [#uses=1]
	ret i16 %0
}

define zeroext i16 @do_crc(i8* %data, i16 zeroext %size) nounwind {
entry:
	%retval = alloca i16		; <i16*> [#uses=2]
	%data.addr = alloca i8*		; <i8**> [#uses=2]
	%size.addr = alloca i16		; <i16*> [#uses=2]
	%crc = alloca i16, align 2		; <i16*> [#uses=4]
	%i = alloca i16, align 2		; <i16*> [#uses=5]
	store i8* %data, i8** %data.addr
	store i16 %size, i16* %size.addr
	store i16 0, i16* %crc
	store i16 0, i16* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp = load i16* %i		; <i16> [#uses=1]
	%conv = zext i16 %tmp to i32		; <i32> [#uses=1]
	%tmp1 = load i16* %size.addr		; <i16> [#uses=1]
	%conv2 = zext i16 %tmp1 to i32		; <i32> [#uses=1]
	%cmp = icmp slt i32 %conv, %conv2		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp4 = load i16* %crc		; <i16> [#uses=1]
	%tmp5 = load i16* %i		; <i16> [#uses=1]
	%tmp6 = load i8** %data.addr		; <i8*> [#uses=1]
	%idxprom = zext i16 %tmp5 to i32		; <i32> [#uses=1]
	%arrayidx = getelementptr i8* %tmp6, i32 %idxprom		; <i8*> [#uses=1]
	%tmp7 = load i8* %arrayidx		; <i8> [#uses=1]
	%call = call zeroext i16 @crc_xmodem_update(i16 zeroext %tmp4, i8 zeroext %tmp7)		; <i16> [#uses=1]
	store i16 %call, i16* %crc
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp8 = load i16* %i		; <i16> [#uses=1]
	%inc = add i16 %tmp8, 1		; <i16> [#uses=1]
	store i16 %inc, i16* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp9 = load i16* %crc		; <i16> [#uses=1]
	store i16 %tmp9, i16* %retval
	%0 = load i16* %retval		; <i16> [#uses=1]
	ret i16 %0
}

define zeroext i16 @do_crc_update(i16 zeroext %crc, i8* %data, i16 zeroext %size) nounwind {
entry:
	%retval = alloca i16		; <i16*> [#uses=2]
	%crc.addr = alloca i16		; <i16*> [#uses=4]
	%data.addr = alloca i8*		; <i8**> [#uses=2]
	%size.addr = alloca i16		; <i16*> [#uses=2]
	%i = alloca i16, align 2		; <i16*> [#uses=5]
	store i16 %crc, i16* %crc.addr
	store i8* %data, i8** %data.addr
	store i16 %size, i16* %size.addr
	store i16 0, i16* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp = load i16* %i		; <i16> [#uses=1]
	%conv = zext i16 %tmp to i32		; <i32> [#uses=1]
	%tmp1 = load i16* %size.addr		; <i16> [#uses=1]
	%conv2 = zext i16 %tmp1 to i32		; <i32> [#uses=1]
	%cmp = icmp slt i32 %conv, %conv2		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp4 = load i16* %crc.addr		; <i16> [#uses=1]
	%tmp5 = load i16* %i		; <i16> [#uses=1]
	%tmp6 = load i8** %data.addr		; <i8*> [#uses=1]
	%idxprom = zext i16 %tmp5 to i32		; <i32> [#uses=1]
	%arrayidx = getelementptr i8* %tmp6, i32 %idxprom		; <i8*> [#uses=1]
	%tmp7 = load i8* %arrayidx		; <i8> [#uses=1]
	%call = call zeroext i16 @crc_xmodem_update(i16 zeroext %tmp4, i8 zeroext %tmp7)		; <i16> [#uses=1]
	store i16 %call, i16* %crc.addr
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp8 = load i16* %i		; <i16> [#uses=1]
	%inc = add i16 %tmp8, 1		; <i16> [#uses=1]
	store i16 %inc, i16* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp9 = load i16* %crc.addr		; <i16> [#uses=1]
	store i16 %tmp9, i16* %retval
	%0 = load i16* %retval		; <i16> [#uses=1]
	ret i16 %0
}

define i32 @main(i32 %argc, i8** %argv) nounwind {
entry:
	%retval = alloca i32		; <i32*> [#uses=2]
	%argc.addr = alloca i32		; <i32*> [#uses=3]
	%argv.addr = alloca i8**		; <i8***> [#uses=8]
	%handle = alloca %struct.usb_dev_handle*, align 4		; <%struct.usb_dev_handle**> [#uses=7]
	%rawVid = alloca [2 x i8], align 1		; <[2 x i8]*> [#uses=4]
	%rawPid = alloca [2 x i8], align 1		; <[2 x i8]*> [#uses=4]
	%vendor = alloca [11 x i8], align 1		; <[11 x i8]*> [#uses=12]
	%product = alloca [8 x i8], align 1		; <[8 x i8]*> [#uses=11]
	%cnt = alloca i32, align 4		; <i32*> [#uses=9]
	%vid = alloca i32, align 4		; <i32*> [#uses=4]
	%pid = alloca i32, align 4		; <i32*> [#uses=4]
	%cnt_crc = alloca i32, align 4		; <i32*> [#uses=6]
	%read_buffer = alloca i8*, align 4		; <i8**> [#uses=4]
	%crc_buffer = alloca i8*, align 4		; <i8**> [#uses=5]
	%addr = alloca i32, align 4		; <i32*> [#uses=11]
	%addr_lo = alloca i16, align 2		; <i16*> [#uses=7]
	%addr_hi = alloca i16, align 2		; <i16*> [#uses=7]
	%step = alloca i16, align 2		; <i16*> [#uses=6]
	%crc = alloca i16, align 2		; <i16*> [#uses=3]
	%bank = alloca i8, align 1		; <i8*> [#uses=4]
	%fp = alloca %struct.__sFILE*, align 4		; <%struct.__sFILE**> [#uses=3]
	store i32 %argc, i32* %argc.addr
	store i8** %argv, i8*** %argv.addr
	store %struct.usb_dev_handle* null, %struct.usb_dev_handle** %handle
	%.array = getelementptr [2 x i8]* %rawVid, i32 0, i32 0		; <i8*> [#uses=1]
	store i8 -64, i8* %.array
	%.array1 = getelementptr [2 x i8]* %rawVid, i32 0, i32 1		; <i8*> [#uses=1]
	store i8 22, i8* %.array1
	%.array2 = getelementptr [2 x i8]* %rawPid, i32 0, i32 0		; <i8*> [#uses=1]
	store i8 -36, i8* %.array2
	%.array3 = getelementptr [2 x i8]* %rawPid, i32 0, i32 1		; <i8*> [#uses=1]
	store i8 5, i8* %.array3
	%.array4 = getelementptr [11 x i8]* %vendor, i32 0, i32 0		; <i8*> [#uses=1]
	store i8 111, i8* %.array4
	%.array5 = getelementptr [11 x i8]* %vendor, i32 0, i32 1		; <i8*> [#uses=1]
	store i8 112, i8* %.array5
	%.array6 = getelementptr [11 x i8]* %vendor, i32 0, i32 2		; <i8*> [#uses=1]
	store i8 116, i8* %.array6
	%.array7 = getelementptr [11 x i8]* %vendor, i32 0, i32 3		; <i8*> [#uses=1]
	store i8 105, i8* %.array7
	%.array8 = getelementptr [11 x i8]* %vendor, i32 0, i32 4		; <i8*> [#uses=1]
	store i8 120, i8* %.array8
	%.array9 = getelementptr [11 x i8]* %vendor, i32 0, i32 5		; <i8*> [#uses=1]
	store i8 120, i8* %.array9
	%.array10 = getelementptr [11 x i8]* %vendor, i32 0, i32 6		; <i8*> [#uses=1]
	store i8 46, i8* %.array10
	%.array11 = getelementptr [11 x i8]* %vendor, i32 0, i32 7		; <i8*> [#uses=1]
	store i8 111, i8* %.array11
	%.array12 = getelementptr [11 x i8]* %vendor, i32 0, i32 8		; <i8*> [#uses=1]
	store i8 114, i8* %.array12
	%.array13 = getelementptr [11 x i8]* %vendor, i32 0, i32 9		; <i8*> [#uses=1]
	store i8 103, i8* %.array13
	%.array14 = getelementptr [11 x i8]* %vendor, i32 0, i32 10		; <i8*> [#uses=1]
	store i8 0, i8* %.array14
	%.array15 = getelementptr [8 x i8]* %product, i32 0, i32 0		; <i8*> [#uses=1]
	store i8 83, i8* %.array15
	%.array16 = getelementptr [8 x i8]* %product, i32 0, i32 1		; <i8*> [#uses=1]
	store i8 78, i8* %.array16
	%.array17 = getelementptr [8 x i8]* %product, i32 0, i32 2		; <i8*> [#uses=1]
	store i8 69, i8* %.array17
	%.array18 = getelementptr [8 x i8]* %product, i32 0, i32 3		; <i8*> [#uses=1]
	store i8 83, i8* %.array18
	%.array19 = getelementptr [8 x i8]* %product, i32 0, i32 4		; <i8*> [#uses=1]
	store i8 82, i8* %.array19
	%.array20 = getelementptr [8 x i8]* %product, i32 0, i32 5		; <i8*> [#uses=1]
	store i8 65, i8* %.array20
	%.array21 = getelementptr [8 x i8]* %product, i32 0, i32 6		; <i8*> [#uses=1]
	store i8 77, i8* %.array21
	%.array22 = getelementptr [8 x i8]* %product, i32 0, i32 7		; <i8*> [#uses=1]
	store i8 0, i8* %.array22
	store i32 0, i32* %cnt_crc
	store i32 0, i32* %addr
	store i16 0, i16* %addr_lo
	store i16 0, i16* %addr_hi
	store i16 0, i16* %step
	store i16 0, i16* %crc
	store i8 0, i8* %bank
	call void @usb_init()
	%tmp = load i32* %argc.addr		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp, 2		; <i1> [#uses=1]
	br i1 %cmp, label %if.then, label %if.end

if.then:		; preds = %entry
	%tmp23 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx = getelementptr i8** %tmp23, i32 0		; <i8**> [#uses=1]
	%tmp24 = load i8** %arrayidx		; <i8*> [#uses=1]
	call void @usage(i8* %tmp24)
	call void @exit(i32 1) noreturn
	unreachable
		; No predecessors!
	br label %if.end

if.end:		; preds = %0, %entry
	%arraydecay = getelementptr [2 x i8]* %rawVid, i32 0, i32 0		; <i8*> [#uses=1]
	%arrayidx25 = getelementptr i8* %arraydecay, i32 1		; <i8*> [#uses=1]
	%tmp26 = load i8* %arrayidx25		; <i8> [#uses=1]
	%conv = zext i8 %tmp26 to i32		; <i32> [#uses=1]
	%mul = mul i32 %conv, 256		; <i32> [#uses=1]
	%arraydecay27 = getelementptr [2 x i8]* %rawVid, i32 0, i32 0		; <i8*> [#uses=1]
	%arrayidx28 = getelementptr i8* %arraydecay27, i32 0		; <i8*> [#uses=1]
	%tmp29 = load i8* %arrayidx28		; <i8> [#uses=1]
	%conv30 = zext i8 %tmp29 to i32		; <i32> [#uses=1]
	%add = add i32 %mul, %conv30		; <i32> [#uses=1]
	store i32 %add, i32* %vid
	%arraydecay31 = getelementptr [2 x i8]* %rawPid, i32 0, i32 0		; <i8*> [#uses=1]
	%arrayidx32 = getelementptr i8* %arraydecay31, i32 1		; <i8*> [#uses=1]
	%tmp33 = load i8* %arrayidx32		; <i8> [#uses=1]
	%conv34 = zext i8 %tmp33 to i32		; <i32> [#uses=1]
	%mul35 = mul i32 %conv34, 256		; <i32> [#uses=1]
	%arraydecay36 = getelementptr [2 x i8]* %rawPid, i32 0, i32 0		; <i8*> [#uses=1]
	%arrayidx37 = getelementptr i8* %arraydecay36, i32 0		; <i8*> [#uses=1]
	%tmp38 = load i8* %arrayidx37		; <i8> [#uses=1]
	%conv39 = zext i8 %tmp38 to i32		; <i32> [#uses=1]
	%add40 = add i32 %mul35, %conv39		; <i32> [#uses=1]
	store i32 %add40, i32* %pid
	%tmp41 = load i32* %vid		; <i32> [#uses=1]
	%arraydecay42 = getelementptr [11 x i8]* %vendor, i32 0, i32 0		; <i8*> [#uses=1]
	%tmp43 = load i32* %pid		; <i32> [#uses=1]
	%arraydecay44 = getelementptr [8 x i8]* %product, i32 0, i32 0		; <i8*> [#uses=1]
	%call = call i32 @usbOpenDevice(%struct.usb_dev_handle** %handle, i32 %tmp41, i8* %arraydecay42, i32 %tmp43, i8* %arraydecay44, i8* null, %struct.__sFILE* null, %struct.__sFILE* null)		; <i32> [#uses=1]
	%cmp45 = icmp ne i32 %call, 0		; <i1> [#uses=1]
	br i1 %cmp45, label %if.then47, label %if.end53

if.then47:		; preds = %if.end
	%tmp48 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%arraydecay49 = getelementptr [8 x i8]* %product, i32 0, i32 0		; <i8*> [#uses=1]
	%tmp50 = load i32* %vid		; <i32> [#uses=1]
	%tmp51 = load i32* %pid		; <i32> [#uses=1]
	%call52 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp48, i8* getelementptr ([55 x i8]* @"\01LC7", i32 0, i32 0), i8* %arraydecay49, i32 %tmp50, i32 %tmp51)		; <i32> [#uses=0]
	call void @exit(i32 1) noreturn
	unreachable
		; No predecessors!
	br label %if.end53

if.end53:		; preds = %1, %if.end
	%arraydecay54 = getelementptr [8 x i8]* %product, i32 0, i32 0		; <i8*> [#uses=1]
	%tmp55 = load i32* %vid		; <i32> [#uses=1]
	%tmp56 = load i32* %pid		; <i32> [#uses=1]
	%call57 = call i32 (i8*, ...)* @printf(i8* getelementptr ([45 x i8]* @"\01LC8", i32 0, i32 0), i8* %arraydecay54, i32 %tmp55, i32 %tmp56)		; <i32> [#uses=0]
	%tmp58 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx59 = getelementptr i8** %tmp58, i32 1		; <i8**> [#uses=1]
	%tmp60 = load i8** %arrayidx59		; <i8*> [#uses=1]
	%call61 = call i32 @strcasecmp(i8* %tmp60, i8* getelementptr ([7 x i8]* @"\01LC9", i32 0, i32 0))		; <i32> [#uses=1]
	%cmp62 = icmp eq i32 %call61, 0		; <i1> [#uses=1]
	br i1 %cmp62, label %if.then64, label %if.else171

if.then64:		; preds = %if.end53
	%tmp65 = load i32* %argc.addr		; <i32> [#uses=1]
	%cmp66 = icmp slt i32 %tmp65, 3		; <i1> [#uses=1]
	br i1 %cmp66, label %if.then68, label %if.end72

if.then68:		; preds = %if.then64
	%tmp69 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx70 = getelementptr i8** %tmp69, i32 0		; <i8**> [#uses=1]
	%tmp71 = load i8** %arrayidx70		; <i8*> [#uses=1]
	call void @usage(i8* %tmp71)
	call void @exit(i32 1) noreturn
	unreachable
		; No predecessors!
	br label %if.end72

if.end72:		; preds = %2, %if.then64
	%tmp73 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx74 = getelementptr i8** %tmp73, i32 2		; <i8**> [#uses=1]
	%tmp75 = load i8** %arrayidx74		; <i8*> [#uses=1]
	%call76 = call %struct.__sFILE* @fopen(i8* %tmp75, i8* getelementptr ([2 x i8]* @"\01LC10", i32 0, i32 0))		; <%struct.__sFILE*> [#uses=1]
	store %struct.__sFILE* %call76, %struct.__sFILE** %fp
	%tmp77 = load %struct.__sFILE** %fp		; <%struct.__sFILE*> [#uses=1]
	%cmp78 = icmp eq %struct.__sFILE* %tmp77, null		; <i1> [#uses=1]
	br i1 %cmp78, label %if.then80, label %if.end86

if.then80:		; preds = %if.end72
	%tmp81 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%tmp82 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx83 = getelementptr i8** %tmp82, i32 2		; <i8**> [#uses=1]
	%tmp84 = load i8** %arrayidx83		; <i8*> [#uses=1]
	%call85 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp81, i8* getelementptr ([21 x i8]* @"\01LC11", i32 0, i32 0), i8* %tmp84)		; <i32> [#uses=0]
	call void @exit(i32 1) noreturn
	unreachable
		; No predecessors!
	br label %if.end86

if.end86:		; preds = %3, %if.end72
	%call87 = call i8* @malloc(i32 1024)		; <i8*> [#uses=1]
	store i8* %call87, i8** %read_buffer
	%call88 = call i8* @malloc(i32 32768)		; <i8*> [#uses=1]
	store i8* %call88, i8** %crc_buffer
	%tmp89 = load i8** %crc_buffer		; <i8*> [#uses=1]
	%call90 = call i8* @memset(i8* %tmp89, i32 0, i32 32768)		; <i8*> [#uses=0]
	store i32 0, i32* %addr
	%tmp91 = load %struct.usb_dev_handle** %handle		; <%struct.usb_dev_handle*> [#uses=1]
	%call92 = call i32 @usb_control_msg(%struct.usb_dev_handle* %tmp91, i32 64, i32 0, i32 0, i32 0, i8* null, i32 0, i32 5000)		; <i32> [#uses=0]
	br label %while.cond

while.cond:		; preds = %if.end148, %if.end86
	%tmp93 = load i8** %read_buffer		; <i8*> [#uses=1]
	%tmp94 = load %struct.__sFILE** %fp		; <%struct.__sFILE*> [#uses=1]
	%call95 = call i32 @fread(i8* %tmp93, i32 1024, i32 1, %struct.__sFILE* %tmp94)		; <i32> [#uses=2]
	store i32 %call95, i32* %cnt
	%cmp96 = icmp sgt i32 %call95, 0		; <i1> [#uses=1]
	br i1 %cmp96, label %while.body, label %while.end

while.body:		; preds = %while.cond
	store i16 0, i16* %step
	br label %for.cond

for.cond:		; preds = %for.inc, %while.body
	%tmp98 = load i16* %step		; <i16> [#uses=1]
	%conv99 = zext i16 %tmp98 to i32		; <i32> [#uses=1]
	%cmp100 = icmp slt i32 %conv99, 1024		; <i1> [#uses=1]
	br i1 %cmp100, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp102 = load i32* %addr		; <i32> [#uses=1]
	%and = and i32 %tmp102, 65535		; <i32> [#uses=1]
	%conv103 = trunc i32 %and to i16		; <i16> [#uses=1]
	store i16 %conv103, i16* %addr_lo
	%tmp104 = load i32* %addr		; <i32> [#uses=1]
	%shr = lshr i32 %tmp104, 16		; <i32> [#uses=1]
	%and105 = and i32 %shr, 255		; <i32> [#uses=1]
	%conv106 = trunc i32 %and105 to i16		; <i16> [#uses=1]
	store i16 %conv106, i16* %addr_hi
	%tmp107 = load %struct.usb_dev_handle** %handle		; <%struct.usb_dev_handle*> [#uses=1]
	%tmp108 = load i16* %addr_hi		; <i16> [#uses=1]
	%conv109 = zext i16 %tmp108 to i32		; <i32> [#uses=1]
	%tmp110 = load i16* %addr_lo		; <i16> [#uses=1]
	%conv111 = zext i16 %tmp110 to i32		; <i32> [#uses=1]
	%tmp112 = load i8** %read_buffer		; <i8*> [#uses=1]
	%tmp113 = load i16* %step		; <i16> [#uses=1]
	%conv114 = zext i16 %tmp113 to i32		; <i32> [#uses=1]
	%add.ptr = getelementptr i8* %tmp112, i32 %conv114		; <i8*> [#uses=1]
	%call115 = call i32 @usb_control_msg(%struct.usb_dev_handle* %tmp107, i32 64, i32 1, i32 %conv109, i32 %conv111, i8* %add.ptr, i32 128, i32 5000)		; <i32> [#uses=0]
	%tmp116 = load i32* %addr		; <i32> [#uses=1]
	%add117 = add i32 %tmp116, 128		; <i32> [#uses=1]
	store i32 %add117, i32* %addr
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp118 = load i16* %step		; <i16> [#uses=1]
	%conv119 = zext i16 %tmp118 to i32		; <i32> [#uses=1]
	%add120 = add i32 %conv119, 128		; <i32> [#uses=1]
	%conv121 = trunc i32 %add120 to i16		; <i16> [#uses=1]
	store i16 %conv121, i16* %step
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp122 = load i8** %crc_buffer		; <i8*> [#uses=1]
	%tmp123 = load i32* %cnt_crc		; <i32> [#uses=1]
	%add.ptr124 = getelementptr i8* %tmp122, i32 %tmp123		; <i8*> [#uses=1]
	%tmp125 = load i8** %read_buffer		; <i8*> [#uses=1]
	%call126 = call i8* @memcpy(i8* %add.ptr124, i8* %tmp125, i32 1024)		; <i8*> [#uses=0]
	%tmp127 = load i32* %cnt_crc		; <i32> [#uses=1]
	%add128 = add i32 %tmp127, 1024		; <i32> [#uses=1]
	store i32 %add128, i32* %cnt_crc
	%tmp129 = load i32* %cnt_crc		; <i32> [#uses=1]
	%cmp130 = icmp sge i32 %tmp129, 32768		; <i1> [#uses=1]
	br i1 %cmp130, label %if.then132, label %if.end148

if.then132:		; preds = %for.end
	%tmp133 = load i8** %crc_buffer		; <i8*> [#uses=1]
	%call134 = call zeroext i16 @do_crc(i8* %tmp133, i16 zeroext -32768)		; <i16> [#uses=1]
	store i16 %call134, i16* %crc
	%tmp135 = load i32* %addr		; <i32> [#uses=1]
	%tmp136 = load i8* %bank		; <i8> [#uses=1]
	%conv137 = zext i8 %tmp136 to i32		; <i32> [#uses=1]
	%tmp138 = load i16* %addr_hi		; <i16> [#uses=1]
	%conv139 = zext i16 %tmp138 to i32		; <i32> [#uses=1]
	%tmp140 = load i16* %addr_lo		; <i16> [#uses=1]
	%conv141 = zext i16 %tmp140 to i32		; <i32> [#uses=1]
	%tmp142 = load i16* %crc		; <i16> [#uses=1]
	%conv143 = zext i16 %tmp142 to i32		; <i32> [#uses=1]
	%call144 = call i32 (i8*, ...)* @printf(i8* getelementptr ([70 x i8]* @"\01LC12", i32 0, i32 0), i32 %tmp135, i32 %conv137, i32 %conv139, i32 %conv141, i32 %conv143)		; <i32> [#uses=0]
	%tmp145 = load i8** %crc_buffer		; <i8*> [#uses=1]
	%call146 = call i8* @memset(i8* %tmp145, i32 0, i32 32768)		; <i8*> [#uses=0]
	%tmp147 = load i8* %bank		; <i8> [#uses=1]
	%inc = add i8 %tmp147, 1		; <i8> [#uses=1]
	store i8 %inc, i8* %bank
	store i32 0, i32* %cnt_crc
	br label %if.end148

if.end148:		; preds = %if.then132, %for.end
	br label %while.cond

while.end:		; preds = %while.cond
	%tmp149 = load %struct.usb_dev_handle** %handle		; <%struct.usb_dev_handle*> [#uses=1]
	%tmp150 = load i16* %addr_hi		; <i16> [#uses=1]
	%conv151 = zext i16 %tmp150 to i32		; <i32> [#uses=1]
	%tmp152 = load i16* %addr_lo		; <i16> [#uses=1]
	%conv153 = zext i16 %tmp152 to i32		; <i32> [#uses=1]
	%call154 = call i32 @usb_control_msg(%struct.usb_dev_handle* %tmp149, i32 64, i32 4, i32 %conv151, i32 %conv153, i8* null, i32 0, i32 5000)		; <i32> [#uses=1]
	store i32 %call154, i32* %cnt
	%tmp155 = load i32* %cnt		; <i32> [#uses=1]
	%cmp156 = icmp slt i32 %tmp155, 1		; <i1> [#uses=1]
	br i1 %cmp156, label %if.then158, label %if.end170

if.then158:		; preds = %while.end
	%tmp159 = load i32* %cnt		; <i32> [#uses=1]
	%cmp160 = icmp slt i32 %tmp159, 0		; <i1> [#uses=1]
	br i1 %cmp160, label %if.then162, label %if.else

if.then162:		; preds = %if.then158
	%tmp163 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%call164 = call i8* @usb_strerror()		; <i8*> [#uses=1]
	%call165 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp163, i8* getelementptr ([15 x i8]* @"\01LC13", i32 0, i32 0), i8* %call164)		; <i32> [#uses=0]
	br label %if.end169

if.else:		; preds = %if.then158
	%tmp166 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%tmp167 = load i32* %cnt		; <i32> [#uses=1]
	%call168 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp166, i8* getelementptr ([25 x i8]* @"\01LC14", i32 0, i32 0), i32 %tmp167)		; <i32> [#uses=0]
	br label %if.end169

if.end169:		; preds = %if.else, %if.then162
	br label %if.end170

if.end170:		; preds = %if.end169, %while.end
	br label %if.end216

if.else171:		; preds = %if.end53
	%tmp172 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx173 = getelementptr i8** %tmp172, i32 1		; <i8**> [#uses=1]
	%tmp174 = load i8** %arrayidx173		; <i8*> [#uses=1]
	%call175 = call i32 @strcasecmp(i8* %tmp174, i8* getelementptr ([4 x i8]* @"\01LC15", i32 0, i32 0))		; <i32> [#uses=1]
	%cmp176 = icmp eq i32 %call175, 0		; <i1> [#uses=1]
	br i1 %cmp176, label %if.then178, label %if.else211

if.then178:		; preds = %if.else171
	store i32 0, i32* %addr
	%tmp179 = load i32* %addr		; <i32> [#uses=1]
	%and180 = and i32 %tmp179, 65535		; <i32> [#uses=1]
	%conv181 = trunc i32 %and180 to i16		; <i16> [#uses=1]
	store i16 %conv181, i16* %addr_lo
	%tmp182 = load i32* %addr		; <i32> [#uses=1]
	%shr183 = lshr i32 %tmp182, 16		; <i32> [#uses=1]
	%and184 = and i32 %shr183, 255		; <i32> [#uses=1]
	%conv185 = trunc i32 %and184 to i16		; <i16> [#uses=1]
	store i16 %conv185, i16* %addr_hi
	%tmp186 = load i32* %addr		; <i32> [#uses=1]
	%call187 = call i32 (i8*, ...)* @printf(i8* getelementptr ([30 x i8]* @"\01LC16", i32 0, i32 0), i32 %tmp186)		; <i32> [#uses=0]
	%tmp188 = load %struct.usb_dev_handle** %handle		; <%struct.usb_dev_handle*> [#uses=1]
	%tmp189 = load i16* %addr_hi		; <i16> [#uses=1]
	%conv190 = zext i16 %tmp189 to i32		; <i32> [#uses=1]
	%tmp191 = load i16* %addr_lo		; <i16> [#uses=1]
	%conv192 = zext i16 %tmp191 to i32		; <i32> [#uses=1]
	%call193 = call i32 @usb_control_msg(%struct.usb_dev_handle* %tmp188, i32 64, i32 5, i32 %conv190, i32 %conv192, i8* null, i32 8192, i32 5000)		; <i32> [#uses=1]
	store i32 %call193, i32* %cnt
	%tmp194 = load i32* %cnt		; <i32> [#uses=1]
	%cmp195 = icmp slt i32 %tmp194, 1		; <i1> [#uses=1]
	br i1 %cmp195, label %if.then197, label %if.end210

if.then197:		; preds = %if.then178
	%tmp198 = load i32* %cnt		; <i32> [#uses=1]
	%cmp199 = icmp slt i32 %tmp198, 0		; <i1> [#uses=1]
	br i1 %cmp199, label %if.then201, label %if.else205

if.then201:		; preds = %if.then197
	%tmp202 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%call203 = call i8* @usb_strerror()		; <i8*> [#uses=1]
	%call204 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp202, i8* getelementptr ([15 x i8]* @"\01LC13", i32 0, i32 0), i8* %call203)		; <i32> [#uses=0]
	br label %if.end209

if.else205:		; preds = %if.then197
	%tmp206 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%tmp207 = load i32* %cnt		; <i32> [#uses=1]
	%call208 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp206, i8* getelementptr ([25 x i8]* @"\01LC14", i32 0, i32 0), i32 %tmp207)		; <i32> [#uses=0]
	br label %if.end209

if.end209:		; preds = %if.else205, %if.then201
	br label %if.end210

if.end210:		; preds = %if.end209, %if.then178
	br label %if.end215

if.else211:		; preds = %if.else171
	%tmp212 = load i8*** %argv.addr		; <i8**> [#uses=1]
	%arrayidx213 = getelementptr i8** %tmp212, i32 0		; <i8**> [#uses=1]
	%tmp214 = load i8** %arrayidx213		; <i8*> [#uses=1]
	call void @usage(i8* %tmp214)
	call void @exit(i32 1) noreturn
	unreachable
		; No predecessors!
	br label %if.end215

if.end215:		; preds = %4, %if.end210
	br label %if.end216

if.end216:		; preds = %if.end215, %if.end170
	%tmp217 = load %struct.usb_dev_handle** %handle		; <%struct.usb_dev_handle*> [#uses=1]
	%call218 = call i32 @usb_close(%struct.usb_dev_handle* %tmp217)		; <i32> [#uses=0]
	store i32 0, i32* %retval
	%5 = load i32* %retval		; <i32> [#uses=1]
	ret i32 %5
}

declare void @usb_init()

define internal void @usage(i8* %name) nounwind {
entry:
	%name.addr = alloca i8*		; <i8**> [#uses=2]
	store i8* %name, i8** %name.addr
	%tmp = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%call = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp, i8* getelementptr ([8 x i8]* @"\01LC17", i32 0, i32 0))		; <i32> [#uses=0]
	%tmp1 = load %struct.__sFILE** @__stderrp		; <%struct.__sFILE*> [#uses=1]
	%tmp2 = load i8** %name.addr		; <i8*> [#uses=1]
	%call3 = call i32 (%struct.__sFILE*, i8*, ...)* @fprintf(%struct.__sFILE* %tmp1, i8* getelementptr ([31 x i8]* @"\01LC18", i32 0, i32 0), i8* %tmp2)		; <i32> [#uses=0]
	ret void
}

declare void @exit(i32) noreturn

declare i32 @usbOpenDevice(%struct.usb_dev_handle**, i32, i8*, i32, i8*, i8*, %struct.__sFILE*, %struct.__sFILE*)

declare i32 @fprintf(%struct.__sFILE*, i8*, ...)

declare i32 @strcasecmp(i8*, i8*)

declare %struct.__sFILE* @fopen(i8*, i8*)

declare i8* @malloc(i32)

declare i8* @memset(i8*, i32, i32)

declare i32 @usb_control_msg(%struct.usb_dev_handle*, i32, i32, i32, i32, i8*, i32, i32)

declare i32 @fread(i8*, i32, i32, %struct.__sFILE*)

declare i8* @memcpy(i8*, i8*, i32)

declare i8* @usb_strerror()

declare i32 @usb_close(%struct.usb_dev_handle*)
