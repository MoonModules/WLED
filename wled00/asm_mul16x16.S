#if defined(ARDUINO_ARCH_ESP32P4)
.text
.align 4
.global asm_mul16x16
.type   asm_mul16x16,@function
# ESP32-P4 needs -march rv32imafc_zicsr_zifencei_xesppie -mabi ilp32f
# a0 = out_packet, a1 = brightness, a2 = num_loops, a3 = pixelbuffer
asm_mul16x16:
  esp.movx.r.cfg    t6              # Enable aligned data access
  or                t6, t6, 2       # Enable aligned data access
  esp.movx.w.cfg    t6              # Enable aligned data access
  li                t6, 8           # put 8 (eventually for vmul bitshift) in temp register 6
  esp.movx.w.sar    t6              # set the numbers of bits to right-shift from t6
  li                t5, 255         # load 255 into t5 for a comparison
  esp.vldbc.8.ip    q1, a1, 0       # load the "B" value into q1 from a1, broadcasting the same value to all 16 values of q1
  li                t1, 0           # start our loop_num counter t1 at 0
  loop:                             # "loop" label
    beq             t1, a2, exit    # branch to "exit" if loop_num == num_loops 
    esp.vld.128.ip  q0, a3, 16      # load 16 "A" values into q0 from a3, then move the pointer by 16 to get a new batch
    beq             a1, t5, skip    # If brightness (a1) == 255, jump to "skip"
    esp.vmul.u8     q2, q0, q1      # C = A*B (q2 = q0 * q1) then >> by esp.movx.w.sar which we set to 8
    esp.vst.128.ip  q2, a0, 16      # store the 16 "C" values into a0, then move the pointer by 16
    j               end_skip        # jump to "end_skip"
  skip:                             # "skip" label
    esp.vst.128.ip  q0, a0, 16      # just store full brightness (q0 from a3) to packet (a0)
  end_skip:                         # "end_skip" label
    addi            t1, t1, 1       # increment loop_num counter t1
    j               loop            # jump to "loop"
  exit:                             # "exit" label
    ret                             # return
#elif defined(ARDUINO_ARCH_ESP32S3)
.text
.align 4
.global asm_mul16x16
.type   asm_mul16x16,@function
asm_mul16x16:
  # a2 = out_packet, a3 = brightness, a4 = num_loops, a5 = pixelbuffer
  entry             a1, 16          # prepare windowed registers and reserve 16 bytes of stack
  movi.n            a8, 8           # put 8 (eventually for vmul bitshift) in a8
  wsr.sar           a8              # set the numbers of bits to right-shift from a8
  movi.n            a6, 255         # load 255 into a6 for a comparison
  ee.vldbc.8.ip     q1, a3, 0       # load the "B" value into q1 from a1, broadcasting the same value to all 16 values of q1
  movi.n            a7, 0           # start our loop_num counter t1 at 0
  loop:                             # "loop" label
    beq             a7, a4, exit    # branch to "exit" if loop_num == num_loops 
    ee.ld.128.usar.ip q3, a5, 0
    ee.vld.128.ip   q4, a5, 16      # load 16 "A" values into q0 from a3, then move the pointer by 16 to get a new batch
    ee.src.q        q0, q3, q4
    ee.vld.128.ip   q0, a5, 16      # load 16 "A" values into q0 from a3, then move the pointer by 16 to get a new batch
    beq             a3, a6, skip    # If brightness (a1) == 255, jump to "skip"
    ee.vmul.u8      q2, q0, q1      # C = A*B (q2 = q0 * q1) then >> by esp.movx.w.sar which we set to 8
    ee.vst.128.ip   q2, a2, 16      # store the 16 "C" values into packet (a2), then move the pointer by 16
    j               end_skip        # jump to "end_skip"
  skip:                             # "skip" label
    ee.vst.128.ip   q0, a2, 16      # just store full brightness (q0 from a5) to packet (a2)
  end_skip:                         # "end_skip" label
    addi            a7, a7, 1       # increment loop_num counter t1
    j               loop            # jump to "loop"
  exit:                             # "exit" label
    retw.n                          # return
#endif