#ifndef ITEM_ROD_HPP
#define ITEM_ROD_HPP

#include "item.hpp"
#include "spells.hpp"

class Rod: public Item
{
public:
    Rod(ItemDataT* const item_data) :
        Item                    (item_data),
        nr_charge_turns_left_   (0) {}

    virtual ~Rod() {}

    void save() override final;

    void load() override final;

    ConsumeItem activate(Actor* const actor) override final;

    Clr interface_clr() const override final
    {
        return clr_violet;
    }

    void on_std_turn_in_inv(const InvType inv_type) override final;

    std::vector<std::string> descr() const override final;

    void identify(const Verbosity verbosity) override final;

    virtual const std::string real_name() const = 0;

protected:
    virtual std::string descr_identified() const = 0;

    virtual void run_effect() = 0;

    std::string name_inf() const override final;

private:
    int nr_charge_turns_left_;
};

class RodPurgeInvis : public Rod
{
public:
    RodPurgeInvis(ItemDataT* const item_data) :
        Rod(item_data) {}

    ~RodPurgeInvis() {}

    const std::string real_name() const override
    {
        return "Purge Invisible";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device reveals any hidden or invisible "
            "creatures in the area around the user.";
    }

    void run_effect() override;
};

class RodCuring : public Rod
{
public:
    RodCuring(ItemDataT* const item_data) :
        Rod(item_data) {}

    ~RodCuring() {}

    const std::string real_name() const override
    {
        return "Curing";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device cures blindness, poisoning, "
            "infections, disease, and weakening, and restores the users "
            "health by a small amount.";
    }

    void run_effect() override;
};

class RodOpening : public Rod
{
public:
    RodOpening(ItemDataT* const item_data) :
        Rod(item_data) {}

    ~RodOpening() {}

    const std::string real_name() const override
    {
        return "Opening";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device opens all locks, lids and doors in "
            "the surrounding area.";
    }

    void run_effect() override;
};

class RodBless : public Rod
{
public:
    RodBless(ItemDataT* const item_data) :
        Rod(item_data) {}

    ~RodBless() {}

    const std::string real_name() const override
    {
        return "Blessing";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device bends reality in favor of the "
            "user for a while.";
    }

    void run_effect() override;
};

class RodCloudMinds : public Rod
{
public:
    RodCloudMinds(ItemDataT* const item_data) :
        Rod(item_data) {}

    ~RodCloudMinds() {}

    const std::string real_name() const override
    {
        return "Cloud Minds";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device clouds the memories of all "
            "creatures in the area, causing them to forget the presence of "
            "the user.";
    }

    void run_effect() override;
};

namespace rod_handling
{

struct RodLook
{
    std::string name_plain;
    std::string name_a;
    Clr clr;
};

void init();

void save();
void load();

} //rod_handling

#endif // ITEM_ROD_HPP
