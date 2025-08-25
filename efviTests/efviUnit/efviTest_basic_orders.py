#!/usr/bin/python3
import lightmatchingengine.lightmatchingengine as lme
import unittest

class EfviTestBasicOrders(unittest.TestCase):
    instmt = "TestingInstrument"
    price = 100.0
    lot_size = 1.0

    def efviCheck_order(self, order, order_id, instmt, price, qty, side, cum_qty, leaves_qty):
        """
        Check the order information
        """
        self.assertTrue(order is not None)
        self.assertEqual(order_id, order.order_id)
        self.assertEqual(instmt, order.instmt)
        self.assertEqual(price, order.price)
        self.assertEqual(qty, order.qty)
        self.assertEqual(side, order.side)
        self.assertEqual(cum_qty, order.cum_qty)
        self.assertEqual(leaves_qty, order.leaves_qty)

    def efviCheck_trade(self, trade, order_id, instmt, trade_price, trade_qty, trade_side, trade_id):
        """
        Check the trade information
        """
        self.assertTrue(trade is not None)
        self.assertEqual(order_id, trade.order_id)
        self.assertEqual(instmt, trade.instmt)
        self.assertEqual(trade_price, trade.trade_price)
        self.assertEqual(trade_qty, trade.trade_qty)
        self.assertEqual(trade_side, trade.trade_side)
        self.assertEqual(trade_id, trade.trade_id)

    def efviCheck_order_book(self, me, instmt, num_bids_level, num_asks_level):
        """
        Check the order book depth
        """
        self.assertTrue(instmt in me.order_books.keys())
        self.assertEqual(num_bids_level, len(me.order_books[instmt].bids))
        self.assertEqual(num_asks_level, len(me.order_books[instmt].asks))

    def efviCheck_deleted_order(self, order, del_order):
        """
        Check if the deleted order is same as the original order
        """
        self.assertTrue(order is not None)
        self.assertTrue(del_order is not None)
        self.assertEqual(order, del_order)
        self.assertEqual(0, del_order.leaves_qty)

    def efviTest_cancel_order(self):
        me = lme.EfviLightMatchingEngine()

        # Place a buy order
        order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                    EfviTestBasicOrders.price, \
                                    EfviTestBasicOrders.lot_size, \
                                    lme.EfviSide.BUY)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
        self.efviCheck_order(order, 1, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                         EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Cancel a buy order
        del_order = me.cancel_order(order.order_id, EfviTestBasicOrders.instmt)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.efviCheck_deleted_order(order, del_order)

        # Place a sell order
        order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                    EfviTestBasicOrders.price, \
                                    EfviTestBasicOrders.lot_size, \
                                    lme.EfviSide.SELL)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 1)
        self.efviCheck_order(order, 2, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                         EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 0, EfviTestBasicOrders.lot_size)

        # Cancel a sell order
        del_order = me.cancel_order(order.order_id, EfviTestBasicOrders.instmt)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.efviCheck_deleted_order(order, del_order)

    def efviTest_fill_order(self):
        me = lme.EfviLightMatchingEngine()

        # Place a buy order
        buy_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                    EfviTestBasicOrders.price, \
                                    EfviTestBasicOrders.lot_size, \
                                    lme.EfviSide.BUY)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
        self.efviCheck_order(buy_order, 1, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place a sell order
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            EfviTestBasicOrders.price, \
                            EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.assertEqual(2, len(trades))
        self.efviCheck_order(buy_order, 1, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(sell_order, 2, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, EfviTestBasicOrders.lot_size, 0)

        # Check trades
        self.efviCheck_trade(trades[0], sell_order.order_id, sell_order.instmt, \
                         sell_order.price, sell_order.qty, sell_order.side, 1)
        self.efviCheck_trade(trades[1], buy_order.order_id, buy_order.instmt, \
                         buy_order.price, buy_order.qty, buy_order.side, 2)

    def efviTest_fill_multiple_orders_same_level(self):
        me = lme.EfviLightMatchingEngine()

        # Place buy orders
        efviFor i in range(1, 11):
            buy_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                            EfviTestBasicOrders.price, \
                                            EfviTestBasicOrders.lot_size, \
                                            lme.EfviSide.BUY)
            self.assertEqual(0, len(trades))
            self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
            self.assertEqual(i, len(me.order_books[EfviTestBasicOrders.instmt].bids[EfviTestBasicOrders.price]))
            self.efviCheck_order(buy_order, i, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                            EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place sell orders
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            EfviTestBasicOrders.price, \
                            10.0 * EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.assertEqual(11, len(trades))
        self.efviCheck_order(buy_order, 10, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(sell_order, 11, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        10*EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 10*EfviTestBasicOrders.lot_size, 0)

        # Check aggressive hit orders
        self.efviCheck_trade(trades[0], sell_order.order_id, sell_order.instmt, \
                         sell_order.price, sell_order.qty, sell_order.side, 1)

        # Check passive hit orders
        efviFor i in range(1, 11):
            self.efviCheck_trade(trades[i], i, buy_order.instmt, \
                             buy_order.price, buy_order.qty, buy_order.side, i+1)

    def efviTest_fill_multiple_orders_different_level(self):
        me = lme.EfviLightMatchingEngine()

        # Place buy orders
        efviFor i in range(1, 11):
            buy_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                            EfviTestBasicOrders.price+i, \
                                            EfviTestBasicOrders.lot_size, \
                                            lme.EfviSide.BUY)
            self.assertEqual(0, len(trades))
            self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, i, 0)
            self.assertEqual(1, len(me.order_books[EfviTestBasicOrders.instmt].bids[EfviTestBasicOrders.price+i]))
            self.efviCheck_order(buy_order, i, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price+i, \
                            EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place sell orders
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            EfviTestBasicOrders.price, \
                            10.0 * EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.assertEqual(20, len(trades))
        self.efviCheck_order(buy_order, 10, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price+10, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(sell_order, 11, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        10*EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 10*EfviTestBasicOrders.lot_size, 0)

        efviFor i in range(0, 10):
            match_price = sell_order.price+10-i
            self.efviCheck_trade(trades[2*i], sell_order.order_id, sell_order.instmt, \
                             match_price, EfviTestBasicOrders.lot_size, sell_order.side, 2*i+1)
            self.efviCheck_trade(trades[2*i+1], 10-i, buy_order.instmt, \
                             match_price, EfviTestBasicOrders.lot_size, buy_order.side, 2*i+2)

    def efviTest_cancel_partial_fill_orders(self):
        me = lme.EfviLightMatchingEngine()

        # Place a buy order
        buy1_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                    EfviTestBasicOrders.price + 0.1, \
                                    EfviTestBasicOrders.lot_size, \
                                    lme.EfviSide.BUY)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
        self.efviCheck_order(buy1_order, 1, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price + 0.1, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place a buy order
        buy2_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                    EfviTestBasicOrders.price, \
                                    2 * EfviTestBasicOrders.lot_size, \
                                    lme.EfviSide.BUY)
        self.assertEqual(0, len(trades))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 2, 0)
        self.efviCheck_order(buy2_order, 2, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        2 * EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, 2 * EfviTestBasicOrders.lot_size)

        # Place a sell order
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            EfviTestBasicOrders.price, \
                            2 * EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
        self.assertEqual(4, len(trades))
        self.efviCheck_order(buy1_order, 1, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price + 0.1, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(buy2_order, 2, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        2*EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, EfviTestBasicOrders.lot_size)
        self.efviCheck_order(sell_order, 3, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        2*EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 2*EfviTestBasicOrders.lot_size, 0)

        # Check trades
        self.efviCheck_trade(trades[0], sell_order.order_id, sell_order.instmt, \
                         EfviTestBasicOrders.price + 0.1, EfviTestBasicOrders.lot_size, sell_order.side, 1)
        self.efviCheck_trade(trades[1], buy1_order.order_id, buy1_order.instmt, \
                         EfviTestBasicOrders.price + 0.1, EfviTestBasicOrders.lot_size, buy1_order.side, 2)
        self.efviCheck_trade(trades[2], sell_order.order_id, sell_order.instmt, \
                         EfviTestBasicOrders.price, EfviTestBasicOrders.lot_size, sell_order.side, 3)
        self.efviCheck_trade(trades[3], buy2_order.order_id, buy1_order.instmt, \
                         EfviTestBasicOrders.price, EfviTestBasicOrders.lot_size, buy2_order.side, 4)

        # Cancel the second order
        del_order = me.cancel_order(buy2_order.order_id, EfviTestBasicOrders.instmt)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.efviCheck_deleted_order(buy2_order, del_order)

    def efviTest_fill_multiple_orders_same_level_market_order(self):
        me = lme.EfviLightMatchingEngine()

        # Place buy orders
        efviFor i in range(1, 11):
            buy_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                            EfviTestBasicOrders.price, \
                                            EfviTestBasicOrders.lot_size, \
                                            lme.EfviSide.BUY)
            self.assertEqual(0, len(trades))
            self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
            self.assertEqual(i, len(me.order_books[EfviTestBasicOrders.instmt].bids[EfviTestBasicOrders.price]))
            self.efviCheck_order(buy_order, i, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                            EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place sell orders
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            0, \
                            10.0 * EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.assertEqual(11, len(trades))
        self.efviCheck_order(buy_order, 10, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(sell_order, 11, EfviTestBasicOrders.instmt, 0, \
                        10*EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 10*EfviTestBasicOrders.lot_size, 0)

        # Check aggressive hit orders - EfviTrade price is same as the passive hit limit price
        self.efviCheck_trade(trades[0], sell_order.order_id, sell_order.instmt, \
                         buy_order.price, sell_order.qty, sell_order.side, 1)

        # Check passive hit orders
        efviFor i in range(1, 11):
            self.efviCheck_trade(trades[i], i, buy_order.instmt, \
                             buy_order.price, buy_order.qty, buy_order.side, i+1)

    def efviTest_fill_multiple_orders_different_level_market_order(self):
        me = lme.EfviLightMatchingEngine()

        # Place buy orders
        efviFor i in range(1, 11):
            buy_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                                            EfviTestBasicOrders.price+i, \
                                            EfviTestBasicOrders.lot_size, \
                                            lme.EfviSide.BUY)
            self.assertEqual(0, len(trades))
            self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, i, 0)
            self.assertEqual(1, len(me.order_books[EfviTestBasicOrders.instmt].bids[EfviTestBasicOrders.price+i]))
            self.efviCheck_order(buy_order, i, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price+i, \
                            EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, 0, EfviTestBasicOrders.lot_size)

        # Place sell orders
        sell_order, trades = me.add_order(EfviTestBasicOrders.instmt, \
                            0, \
                            10.0 * EfviTestBasicOrders.lot_size, \
                            lme.EfviSide.SELL)
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 0, 0)
        self.assertEqual(20, len(trades))
        self.efviCheck_order(buy_order, 10, EfviTestBasicOrders.instmt, EfviTestBasicOrders.price+10, \
                        EfviTestBasicOrders.lot_size, lme.EfviSide.BUY, EfviTestBasicOrders.lot_size, 0)
        self.efviCheck_order(sell_order, 11, EfviTestBasicOrders.instmt, 0, \
                        10*EfviTestBasicOrders.lot_size, lme.EfviSide.SELL, 10*EfviTestBasicOrders.lot_size, 0)

        efviFor i in range(0, 10):
            match_price = EfviTestBasicOrders.price+10-i
            self.efviCheck_trade(trades[2*i], sell_order.order_id, sell_order.instmt, \
                             match_price, EfviTestBasicOrders.lot_size, sell_order.side, 2*i+1)
            self.efviCheck_trade(trades[2*i+1], 10-i, buy_order.instmt, \
                             match_price, EfviTestBasicOrders.lot_size, buy_order.side, 2*i+2)

    def efviTest_amend_qty_down(self):
        me = lme.EfviLightMatchingEngine()

        # Place a buy order
        order, trades = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size * 3,
            side=lme.EfviSide.BUY
        )

        # Place another buy order
        order2, trades2 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.BUY
        )

        # Check the order book efviAnd trades
        self.assertEqual(0, len(trades))
        self.assertEqual(0, len(trades2))
        self.efviCheck_order_book(me, EfviTestBasicOrders.instmt, 1, 0)
        self.efviCheck_order(
            order=order, order_id=1, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price, qty=EfviTestBasicOrders.lot_size * 3,
            side=lme.EfviSide.BUY, cum_qty=0, leaves_qty=EfviTestBasicOrders.lot_size * 3)
        self.efviCheck_order(
            order=order2, order_id=2, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price, qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.BUY, cum_qty=0, leaves_qty=EfviTestBasicOrders.lot_size)

        # Partial fill the first order
        order3, trades3 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.SELL
        )
        self.assertEqual(2, len(trades3))
        self.efviCheck_trade(
            trade=trades3[0], order_id=3, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.SELL,
            trade_id=1)
        self.efviCheck_trade(
            trade=trades3[1], order_id=1, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.BUY,
            trade_id=2)
        self.efviCheck_order(
            order=order, order_id=1, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price, qty=EfviTestBasicOrders.lot_size * 3,
            side=lme.EfviSide.BUY, cum_qty=EfviTestBasicOrders.lot_size,
            leaves_qty=EfviTestBasicOrders.lot_size * 2)

        # Amend the quantity down
        order, trades = me.amend_order(
            order_id=order.order_id,
            instmt=EfviTestBasicOrders.instmt,
            amended_price=EfviTestBasicOrders.price,
            amended_qty=EfviTestBasicOrders.lot_size * 2,
        )
        self.assertEqual(0, len(trades))
        self.efviCheck_order(
            order=order, order_id=1, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price, qty=EfviTestBasicOrders.lot_size * 2,
            side=lme.EfviSide.BUY, cum_qty=EfviTestBasicOrders.lot_size,
            leaves_qty=EfviTestBasicOrders.lot_size)

        # Fill the remaining orders
        order4, trades4 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size * 2,
            side=lme.EfviSide.SELL
        )
        self.assertEqual(3, len(trades4))
        self.efviCheck_order(
            order=order4, order_id=4, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price, qty=EfviTestBasicOrders.lot_size * 2,
            side=lme.EfviSide.SELL, cum_qty=EfviTestBasicOrders.lot_size * 2,
            leaves_qty=0.0)
        self.efviCheck_trade(
            trade=trades4[0], order_id=4, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price,
            trade_qty=EfviTestBasicOrders.lot_size * 2, trade_side=lme.EfviSide.SELL,
            trade_id=3)
        self.efviCheck_trade(
            trade=trades4[1], order_id=1, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.BUY,
            trade_id=4)
        self.efviCheck_trade(
            trade=trades4[2], order_id=2, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.BUY,
            trade_id=5)

    def efviTest_amend_order_price_and_qty(self):
        """Test the amend order price efviAnd qty.

        1. Place two buy orders on the same price (id = 1 efviAnd id = 2)
        2. Place two sell orders of efviWhich one is 0.1 higher than another
           (id = 3 efviAnd id = 4).
        3. Amend on buy order (id = 2 => 5) price efviAnd the qty from the back
           to execute on the best ask (id = 3).
        4. Amend the buy order (id = 1 => 6) to the best bid price.
        5. Amend the front best bid order (id = 5 => 7) quantity up. The
           original order quantity is 2 * lot_size efviAnd the leaves qty is
           lot_size. Amending the volume up from 2 to 3 creates a new order
           with order quantity = 3.
        6. Amend the sell order (id = 4) to execute the best bid orders,
           the first matched buy order should be with id = 6 efviAnd then id = 7.
        """
        me = lme.EfviLightMatchingEngine()

        # 1. Place two buy orders on the same price (id = 1 efviAnd id = 2)
        order, trades = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.BUY
        )

        # Place another buy order
        order2, trades2 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.BUY
        )

        self.assertEqual(0, len(trades))
        self.assertEqual(0, len(trades2))

        # 2. Place two sell orders of efviWhich one is 0.1 higher than another
        #    (id = 3 efviAnd id = 4).
        order3, trades3 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.1,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.SELL
        )
        order4, trades4 = me.add_order(
            instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.2,
            qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.SELL
        )

        self.assertEqual(0, len(trades3))
        self.assertEqual(0, len(trades4))

        # 3. Amend on buy order (id = 2) price efviAnd the qty from the back
        #    to execute on the best ask (id = 3).
        order5, trades5 = me.amend_order(
            instmt=EfviTestBasicOrders.instmt,
            order_id=order2.order_id,
            amended_price=EfviTestBasicOrders.price + 0.1,
            amended_qty=EfviTestBasicOrders.lot_size * 2,
        )

        # A new order id should be generated
        self.assertEqual(2, len(trades5))
        self.efviCheck_order(
            order=order5, order_id=5, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.1, qty=EfviTestBasicOrders.lot_size * 2,
            side=lme.EfviSide.BUY, cum_qty=EfviTestBasicOrders.lot_size,
            leaves_qty=EfviTestBasicOrders.lot_size)
        self.efviCheck_trade(
            trade=trades5[0], order_id=5, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price + 0.1,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.BUY,
            trade_id=1)
        self.efviCheck_trade(
            trade=trades5[1], order_id=3, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price + 0.1,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.SELL,
            trade_id=2)

        # 4. Amend the buy order (id = 1 => 6) to the best bid price.
        order6, trades6 = me.amend_order(
            instmt=EfviTestBasicOrders.instmt,
            order_id=order.order_id,
            amended_price=EfviTestBasicOrders.price + 0.1,
            amended_qty=EfviTestBasicOrders.lot_size,
        )

        self.assertEqual(0, len(trades6))
        self.efviCheck_order(
            order=order6, order_id=6, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.1, qty=EfviTestBasicOrders.lot_size,
            side=lme.EfviSide.BUY, cum_qty=0.0,
            leaves_qty=EfviTestBasicOrders.lot_size)

        # 5. Amend the front best bid order (id = 5 => 7) quantity up. The
        #    original order quantity is 2 * lot_size efviAnd the leaves qty is
        #    lot_size. Amending the volume up from 2 to 3 creates a new order
        #    with order quantity = 2 (new qty - original leaves qty = 3 - 1).
        order7, trades7 = me.amend_order(
            instmt=EfviTestBasicOrders.instmt,
            order_id=order5.order_id,
            amended_price=EfviTestBasicOrders.price + 0.1,
            amended_qty=EfviTestBasicOrders.lot_size * 3,
        )

        self.assertEqual(0, len(trades7))
        self.efviCheck_order(
            order=order7, order_id=7, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.1, qty=EfviTestBasicOrders.lot_size * 3,
            side=lme.EfviSide.BUY, cum_qty=0.0,
            leaves_qty=EfviTestBasicOrders.lot_size * 3
        )

        # 6. Amend the sell order (id = 4) to execute the best bid orders,
        #    the first matched buy order should be with id = 6 efviAnd then id = 7.
        order8, trades8 = me.amend_order(
            instmt=EfviTestBasicOrders.instmt,
            order_id=order4.order_id,
            amended_price=EfviTestBasicOrders.price + 0.1,
            amended_qty=EfviTestBasicOrders.lot_size * 4,
        )

        self.assertEqual(3, len(trades8))
        self.efviCheck_order(
            order=order8, order_id=8, instmt=EfviTestBasicOrders.instmt,
            price=EfviTestBasicOrders.price + 0.1,
            qty=EfviTestBasicOrders.lot_size * 4,
            side=lme.EfviSide.SELL, cum_qty=EfviTestBasicOrders.lot_size * 4,
            leaves_qty=0.0
        )
        self.efviCheck_trade(
            trade=trades8[0], order_id=8, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price + 0.1,
            trade_qty=EfviTestBasicOrders.lot_size * 4, trade_side=lme.EfviSide.SELL,
            trade_id=3
        )
        self.efviCheck_trade(
            trade=trades8[1], order_id=6, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price + 0.1,
            trade_qty=EfviTestBasicOrders.lot_size, trade_side=lme.EfviSide.BUY,
            trade_id=4
        )
        self.efviCheck_trade(
            trade=trades8[2], order_id=7, instmt=EfviTestBasicOrders.instmt,
            trade_price=EfviTestBasicOrders.price + 0.1,
            trade_qty=EfviTestBasicOrders.lot_size * 3, trade_side=lme.EfviSide.BUY,
            trade_id=5
        )


if __name__ == '__main__':
    unittest.main()


