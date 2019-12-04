#include <cstring>
#include <iostream>
#include "ethercat.h"

uint8_t iomap_[2048];
static inline bool IS_ELMO(int32_t slave_no) {
  //   return ec_slave[slave_no].eep_man == 0x01DD;
  //   to-do: replace the elmo eepman id by yourself
  return true;
}
void init_soem(const std::string& ifname) {
  if (ec_init(ifname.c_str())) {
    if (ec_config_init(FALSE)) {
      if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE * 4) !=
          EC_STATE_PRE_OP) {
        fprintf(stderr, "Could not set EC_STATE_PRE_OP \n");
        return;
      }

      ec_configdc();
      for (int slc = 1; slc <= ec_slavecount; slc++) {
        if (ec_slave[slc].hasdc > 0) {
          ec_dcsync0(slc, TRUE, 1000 * 1000, 0);
        }
      }
      if (!ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4)) {
        fprintf(stderr, "Could not set EC_STATE_SAFE_OP\n");
        return;
      }

      int iomap_size = ec_config_map(&iomap_);
      printf("SOEM IOMap size: %d\n", iomap_size);
      ec_slave[0].state = EC_STATE_OPERATIONAL;
      ec_send_processdata();
      ec_receive_processdata(EC_TIMEOUTRET);

      ec_writestate(0);
      int chk = 40;
      do {
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL,
                      50000);  // 50ms wait for state check
      } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

      if (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE) !=
          EC_STATE_OPERATIONAL) {
        fprintf(stderr, "OPERATIONAL state not set, exiting\n");
        return;
      }

      ec_readstate();

      for (int slc = 1; slc <= ec_slavecount; slc++) {
        if (IS_ELMO(slc)) {
          int ret = 0, len;
          uint16_t sync_mode;
          uint32_t cycle_time;
          uint32_t shift_time;
          uint32_t minimum_cycle_time;
          uint32_t calc_and_copy_time;
          uint32_t sync0_cycle_time;

          len = sizeof(sync_mode);
          ret += ec_SDOread(slc, 0x1c32, 0x01, FALSE, &len, &sync_mode,
                            EC_TIMEOUTRXM);

          len = sizeof(cycle_time);
          ret += ec_SDOread(slc, 0x1c32, 0x02, FALSE, &len, &cycle_time,
                            EC_TIMEOUTRXM);

          len = sizeof(shift_time);
          ret += ec_SDOread(slc, 0x1c32, 0x03, FALSE, &len, &shift_time,
                            EC_TIMEOUTRXM);

          len = sizeof(minimum_cycle_time);
          ret += ec_SDOread(slc, 0x1c32, 0x05, FALSE, &len, &minimum_cycle_time,
                            EC_TIMEOUTRXM);

          len = sizeof(calc_and_copy_time);
          ret += ec_SDOread(slc, 0x1c32, 0x06, FALSE, &len, &calc_and_copy_time,
                            EC_TIMEOUTRXM);

          len = sizeof(sync0_cycle_time);
          ret += ec_SDOread(slc, 0x1c32, 0x0a, FALSE, &len, &sync0_cycle_time,
                            EC_TIMEOUTRXM);
          printf(
              "PDO syncmode %02x, cycle time %d ns (min %d), shift time %d ns, "
              "calc "
              "and copy time %d ns, sync0 cycle time %d ns \n",
              sync_mode, cycle_time, minimum_cycle_time, shift_time,
              calc_and_copy_time, sync0_cycle_time);
        }
      }

      printf("\nFinished configuration successfully\n");
    }
  } else {
    printf("can't bind soem on %s\n", ifname.c_str());
  }

  ec_close();
}

int main() {
  std::string ifname("nic_name");
  init_soem(ifname);
}