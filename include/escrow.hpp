#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <contracts.hpp>
#include <tables/users.hpp>
#include <config.hpp>
#include <util.hpp>
#include <common.hpp>

using namespace eosio;

CONTRACT escrow : public contract {
  
  public:
    using contract::contract;
    escrow(name receiver, name code, datastream<const char*> ds)
      : contract(receiver, code, ds),
        config(contracts::settings, contracts::settings.value)
        {}

    ACTION reset();

    ACTION deposit(const name & from, const name & to, const asset & quantity, const std::string & memo);

    ACTION withdraw(const name & account, const asset & quantity);

    ACTION upsertuser(const name & account, const mapss & contact_methods, const mapss & payment_methods, const name & time_zone, const name & fiat_currency);

    ACTION addselloffer(const name & seller, const asset & total_offered, const uint64_t & price_percentage);

    ACTION cancelsoffer(const uint64_t & sell_offer_id);

    ACTION addbuyoffer(const name & buyer, const uint64_t & sell_offer_id, const asset & quantity, const std::string & payment_method);

    ACTION delbuyoffer(const uint64_t & buy_offer_id);

    ACTION accptbuyoffr(const uint64_t & buy_offer_id);

    ACTION payoffer(const uint64_t & buy_offer_id);

    ACTION confrmpaymnt(const uint64_t & buy_offer_id);

  private:

    const name offer_type_sell = name("offer.sell");
    const name offer_type_buy = name("offer.buy"); 

    const name sell_offer_status_active = name("s.active");
    const name sell_offer_status_soldout = name("s.soldout");
    const name sell_offer_status_canceled = name("s.canceled");

    const name buy_offer_status_pending = name("b.pending");
    const name buy_offer_status_accepted = name("b.accepted");
    const name buy_offer_status_paid = name("b.paid");
    const name buy_offer_status_confirmed = name("b.confirmd");
    const name buy_offer_status_successful = name("b.success");

    DEFINE_CONFIG_TABLE
    DEFINE_CONFIG_GET

    DEFINE_USERS_TABLE

    TABLE balances_table {
      name account;
      asset available_balance;
      asset swap_balance;
      asset escrow_balance;

      uint64_t primary_key () const { return account.value; }
    };

    TABLE transactions_stats_table {
      name account;
      uint64_t total_trx;
      uint64_t sell_successful;
      uint64_t buy_successful; // does it make any difference?
    
      uint64_t primary_key () const { return account.value; }
      uint128_t by_total_account () const { return (uint128_t(total_trx) << 64) + account.value; }
      uint128_t by_sell_account () const { return (uint128_t(sell_successful) << 64) + account.value; }
      uint128_t by_buy_account () const { return (uint128_t(buy_successful) << 64) + account.value; }
    };

    TABLE offer_table {
      uint64_t id;
      name seller;
      name buyer;
      name type;
      mapna quantity_info;
      mapnui64 price_info;
      time_point created_date;
      mapnt status_history;
      mapss payment_methods;
      name current_status;
      name time_zone;
      name fiat_currency;

      uint64_t primary_key () const { return id; }
      uint128_t by_type_id () const { return (uint128_t(type.value) << 64) + id; }
      uint128_t by_seller_id () const { return (uint128_t(seller.value) << 64) + id; }
      uint128_t by_seller_date () const { return (uint128_t(seller.value) << 64) + (std::numeric_limits<uint64_t>::max() - created_date.sec_since_epoch()); }
      uint128_t by_buyer_id () const { return (uint128_t(buyer.value) << 64) + id; }
      uint128_t by_buyer_date () const { return (uint128_t(buyer.value) << 64) + (std::numeric_limits<uint64_t>::max() - created_date.sec_since_epoch()); }
      uint128_t by_current_status_seller () const { return (uint128_t(current_status.value) << 64) + seller.value; }
      uint128_t by_current_status_buyer () const { return (uint128_t(current_status.value) << 64) + buyer.value; }
      uint128_t by_current_status_id () const { return (uint128_t(current_status.value) << 64) + id; }
    };

    typedef eosio::multi_index<name("offers"), offer_table,
      indexed_by<name("bytypeid"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_type_id>>,
      indexed_by<name("bysellerid"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_seller_id>>,
      indexed_by<name("bysellerdate"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_seller_date>>,
      indexed_by<name("bybuyerid"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_buyer_id>>,
      indexed_by<name("bybuyerdate"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_buyer_date>>,
      indexed_by<name("bycstatuss"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_current_status_seller>>,
      indexed_by<name("bycstatusb"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_current_status_buyer>>,
      indexed_by<name("bycstatusid"),
      const_mem_fun<offer_table, uint128_t, &offer_table::by_current_status_id>>
    > offer_tables;

    // TABLE sell_offer_table {
    //   uint64_t id;
    //   name seller;
    //   asset total_offered;
    //   asset available_quantity;
    //   uint64_t price_percentage;
    //   time_point created_date;
    //   name time_zone;
    //   name fiat_currency;
    //   mapss payment_methods;
    //   std::string additional_info;
    //   // status ?

    //   uint64_t primary_key () const { return id; }
    //   uint64_t by_seller () const { return seller.value; }
    //   uint64_t by_price_percentage () const { return price_percentage; }
    //   uint64_t by_created_date () const { return created_date.sec_since_epoch(); }
    //   uint64_t by_time_zone () const { return time_zone.value; }
    //   uint64_t by_fiat_currency () const { return fiat_currency.value; }
    // };

    // TABLE buy_offer_table {
    //   uint64_t id;
    //   name buyer;
    //   name seller;
    //   asset quantity;
    //   uint64_t price_percentage;
    //   time_point created_date;
    //   mapnt status_history;
    //   std::vector<std::string> payment_method;
    //   name status;

    //   uint64_t primary_key () const { return id; }
    //   uint64_t by_buyer () const { return buyer.value; }
    //   uint64_t by_seller () const { return seller.value; }
    //   uint64_t by_created_date () const { return created_date.sec_since_epoch(); }
    //   uint64_t by_status () const { return status.value; }
    //   uint128_t by_buyer_seller () const { return (uint128_t(buyer.value) << 64) + seller.value; }
    //   uint128_t by_seller_buyer () const { return (uint128_t(seller.value) << 64) + buyer.value; }
    //   uint128_t by_status_seller () const { return (uint128_t(status.value) << 64) + seller.value; }
    //   uint128_t by_status_buyer () const { return (uint128_t(status.value) << 64) + buyer.value; }
    // };

    TABLE buy_sell_relation_table {
      uint64_t id;
      uint64_t sell_offer_id;
      uint64_t buy_offer_id;

      uint64_t primary_key () const { return id; }
      uint64_t by_buy () const { return buy_offer_id; }
      uint128_t by_sell_buy () const { return (uint128_t(sell_offer_id) << 64) + buy_offer_id; }
    };

    typedef eosio::multi_index<name("balances"), balances_table> balances_tables;

    typedef eosio::multi_index<name("trxstats"), transactions_stats_table,
      indexed_by<name("bytotalacct"),
      const_mem_fun<transactions_stats_table, uint128_t, &transactions_stats_table::by_total_account>>,
      indexed_by<name("bysellacct"),
      const_mem_fun<transactions_stats_table, uint128_t, &transactions_stats_table::by_sell_account>>,
      indexed_by<name("bybuyacct"),
      const_mem_fun<transactions_stats_table, uint128_t, &transactions_stats_table::by_buy_account>>
    > transactions_stats_tables;

    // typedef eosio::multi_index<name("selloffers"), sell_offer_table,
    //   indexed_by<name("byseller"),
    //   const_mem_fun<sell_offer_table, uint64_t, &sell_offer_table::by_seller>>,
    //   indexed_by<name("bypriceper"),
    //   const_mem_fun<sell_offer_table, uint64_t, &sell_offer_table::by_price_percentage>>,
    //   indexed_by<name("bydate"),
    //   const_mem_fun<sell_offer_table, uint64_t, &sell_offer_table::by_created_date>>,
    //   indexed_by<name("bytimezone"),
    //   const_mem_fun<sell_offer_table, uint64_t, &sell_offer_table::by_time_zone>>,
    //   indexed_by<name("byfiatcrrncy"),
    //   const_mem_fun<sell_offer_table, uint64_t, &sell_offer_table::by_fiat_currency>>
    // > sell_offer_tables;

    // typedef eosio::multi_index<name("buyoffers"), buy_offer_table,
    //   indexed_by<name("bybuyer"),
    //   const_mem_fun<buy_offer_table, uint64_t, &buy_offer_table::by_buyer>>,
    //   indexed_by<name("byseller"),
    //   const_mem_fun<buy_offer_table, uint64_t, &buy_offer_table::by_seller>>,
    //   indexed_by<name("bydate"),
    //   const_mem_fun<buy_offer_table, uint64_t, &buy_offer_table::by_created_date>>,
    //   indexed_by<name("bystatus"),
    //   const_mem_fun<buy_offer_table, uint64_t, &buy_offer_table::by_seller>>,
    //   indexed_by<name("bybuyrsellr"),
    //   const_mem_fun<buy_offer_table, uint128_t, &buy_offer_table::by_buyer_seller>>,
    //   indexed_by<name("bysellrbuyr"),
    //   const_mem_fun<buy_offer_table, uint128_t, &buy_offer_table::by_seller_buyer>>,
    //   indexed_by<name("bystatussllr"),
    //   const_mem_fun<buy_offer_table, uint128_t, &buy_offer_table::by_status_seller>>,
    //   indexed_by<name("bystatusbuyr"),
    //   const_mem_fun<buy_offer_table, uint128_t, &buy_offer_table::by_status_buyer>>
    // > buy_offer_tables;

    typedef eosio::multi_index<name("buysellrel"), buy_sell_relation_table,
      indexed_by<name("bybuy"),
      const_mem_fun<buy_sell_relation_table, uint64_t, &buy_sell_relation_table::by_buy>>,
      indexed_by<name("bysellbuy"),
      const_mem_fun<buy_sell_relation_table, uint128_t, &buy_sell_relation_table::by_sell_buy>>
    > buy_sell_relation_tables;


    config_tables config;

    void send_transfer(const name & beneficiary, const asset & quantity, const std::string & memo);

};

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  if (action == name("transfer").value && code == seeds::token.value) {
      execute_action<escrow>(name(receiver), name(code), &escrow::deposit);
  } else if (code == receiver) {
      switch (action) {
          EOSIO_DISPATCH_HELPER(escrow, 
          (reset)(withdraw)
          (upsertuser)
          (addselloffer)(cancelsoffer)
          (addbuyoffer)(delbuyoffer)
          (accptbuyoffr)(payoffer)(confrmpaymnt)
        )
      }
  }
}
