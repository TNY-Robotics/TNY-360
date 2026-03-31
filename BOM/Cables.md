# Cables

Here's a list of all cables needed to wire up your TNY-360.

A complete kit of JST-PH wires is also linked below for easier sourcing, instead of buying each cable independently.

### Assembly guide

To learn how to fully assemble you TNY-360, check out the [TNY-360 Documentation](https://tny-robotics.com/docs/tny-360/practical-guide/sourcing/). A more user-friendly list of all the cables needed is also available here.

| Type             | Pins | Length | Quantity | Location                         | Link                                                               |
| :--------------- | :--: | :----- | :------: | :------------------------------- | :----------------------------------------------------------------- |
| JST-PH           |  3   | 100 mm |    4     | Motor Driver -> Paw Board        | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH           |  2   | 100 mm |    1     | Motor Driver -> Main Board       | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH           |  5   | 100 mm |    1     | Motor Driver -> Main Board       | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH           |  4   | 100 mm |    4     | Analog Reader -> Paw Board       | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH           |  7   | 100 mm |    1     | Analog Reader -> Main Board      | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH (PIGTAIL) |  4   | 200 mm |    1     | IMU -> Paw Board                 | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-PH           |  4   | 200 mm |    2     | Plug Board -> Power & Main Board | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH           |  2   | 200 mm |    2     | Plug Board -> Power & Main Board | [Click here](https://s.click.aliexpress.com/e/_c3BIp6Ux)           |
| JST-PH (PIGTAIL) |  4   | 50 mm  |    1     | SH1106 -> Main Board             | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-PH (PIGTAIL) |  3   | 50 mm  |    2     | TTP23 -> Main Board              | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-PH (PIGTAIL) |  4   | 50 mm  |    1     | VL53L0X -> Main Board            | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-PH (PIGTAIL) |  5   | 50 mm  |    1     | I2S Mic. -> Main Board           | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-PH (PIGTAIL) |  2   | 450 mm |    4     | Contact Switch -> Paw Board      | [Click here](https://s.click.aliexpress.com/e/_c44pILQb)           |
| JST-XH (PIGTAIL) |  4   | 100 mm |    8     | Shoulder Servo -> Paw Board      | [Click here](https://s.click.aliexpress.com/e/_c3WRxkg7)           |
| JST-XH (PIGTAIL) |  4   | 300 mm |    4     | Leg Servo -> Paw Board           | [Click here](https://s.click.aliexpress.com/e/_c3WRxkg7)           |
| JST-XH (PIGTAIL) |  4   | 300 mm |    4     | Leg Servo -> Paw Board           | [Click here](https://s.click.aliexpress.com/e/_c3WRxkg7)           |
| 16 AWG Wire      |      | 100 mm |    3     | DC-DC Converter -> Paw Board     | [Click here](https://fr.aliexpress.com/item/1005006614755156.html) |

### Connectors
| Type                 | Quantity | Link                                                     |
| -------------------- | -------- | -------------------------------------------------------- |
| Ring Crimp Connector | 6        | [Click here](https://s.click.aliexpress.com/e/_c2yxJCd5) |
| Dupont Connector     | 4        | None - Use the ones from the MG996R motors :)            |

### Complete kit

To simplify cable sourcing, you can alternatively buy only one JST-PH cable kit, and some JST-XH pigtail cables.

* Link to the [JST-PH Cables Kit](https://s.click.aliexpress.com/e/_c3AtPZ9h).
* Link to the [Dupont Connector Kit](https://fr.aliexpress.com/item/4001063470283.html);
* Link to the [JST-XH Pigtail Cables](https://s.click.aliexpress.com/e/_c3WRxkg7).

Please check that you selected the `XH-2.54mm version (4P)` before ordering.

### About the SH1106, TTP23, VL53L0X and I2S Mic. cables

Since these cables are going to be soldered directly on the component, you can buy a 100mm cable and cut it in half to make 2x 50mm pigtail cables. This reduces the number of cables to order.

### About the contact switch cable

The contact switch cable should end with a 3-pin Dupont connector (the standard 2.54mm arduino connectors), with the middle pin empty.

No such cable (JST-PH 2-Pins to 2.54 mm Dupont 3 Pins) exists on the web, so you will probably have to make it yourself. A complete guide on how to create that cable is available *(or will be available)* on the [TNY-360 Documentation](https://tny-robotics.com/docs/tny-360).
