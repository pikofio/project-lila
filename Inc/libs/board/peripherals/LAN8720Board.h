//here live implementation for RJ45 board peripheral

#include "gpio.h"

typedef struct {
    uint16_t BCR,       //Basic Control Register                            index[0]
    uint16_t BSR,       //Basic Status Register                             index[1]
    uint16_t PHYIDR1,   //PHYsical IDentifier Register 1                    index[2]
    uint16_t PHYIDR2,   //PHYsical IDentifier Register 2                    index[3]
    uint16_t ANAR,      //Auto-Negotiation Advertisement Register           index[4]
    uint16_t ANLPAR,    //Auto-Negotiation Link Partner Ability Register    index[5]
    uint16_t ANER,      //Auto-Negotiation Expansion Register               index[6]
    uint16_t MCSR,      //Mode Control/Status Register                      index[17]
    uint16_t SMR,       //Special Modes                                     index[18]
    uint16_t SECR,      //Symbol Error Counter Register                     index[26]
    uint16_t CSIR,      //Control / Status Indication Register              index[27]
    uint16_t ISR,       //Interrupt Source Register                         index[29]
    uint16_t IMR,       //Interrupt Mask Register                           index[30]
    uint16_t PHYSCSR    //PHY Special Control/Status Register               index[31]
}LAN_8720_TypeDef;

#define BCR_BASE        (0U)
#define BSR_BASE        (1U)
#define PHYIDR1_BASE    (2U)
#define PHYIDR2_BASE    (3U)
#define ANAR_BASE       (4U)
#define ANLPAR_BASE     (5U)
#define ANER_BASE       (6U)
#define MCSR_BASE       (17U)
#define SMR_BASE        (18U)
#define SECR_BASE       (26U)
#define CSIR_BASE       (27U)
#define ISR_BASE        (29U)
#define IMR_BASE        (30U)
#define PHYSCSR_BASE    (31U)


void lan8720_init(void)
{
    pin_setup(PC03, ALTERNATE, PUSH_PULL, VERY_HIGH, NO_PUPU); //MDC pin setup
    pin_set_alternate(PC03, ETH_);

    pin_setup(PA01, ALTERNATE, OPEN_DRAIN, VERY_HIGH, PULL_UP);  //MDIO pin setup
    pin_set_alternate(PA01, ETH_);

}

uint16_t lan8720_read()
{
    
}