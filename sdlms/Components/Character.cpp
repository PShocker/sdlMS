#include "Character.h"
#include "Sprite.h"
#include "Transform.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <tuple>

#include "Resources/Wz.h"
#include "Commons/Commons.h"

Character::Character()
{
    zmap[u"mobEquipFront"] = &mobEquipFront;
    zmap[u"tamingMobFront"] = &tamingMobFront;
    zmap[u"mobEquipMid"] = &mobEquipMid;
    zmap[u"saddleFront"] = &saddleFront;
    zmap[u"mobEquipUnderSaddle"] = &mobEquipUnderSaddle;
    zmap[u"tamingMobMid"] = &tamingMobMid;
    zmap[u"characterStart"] = &characterStart;
    zmap[u"emotionOverBody"] = &emotionOverBody;
    zmap[u"weaponWristOverGlove"] = &weaponWristOverGlove;
    zmap[u"capeOverHead"] = &capeOverHead;
    zmap[u"weaponOverGlove"] = &weaponOverGlove;
    zmap[u"gloveWristOverHair"] = &gloveWristOverHair;
    zmap[u"gloveOverHair"] = &gloveOverHair;
    zmap[u"handOverHair"] = &handOverHair;
    zmap[u"weaponOverHand"] = &weaponOverHand;
    zmap[u"shieldOverHair"] = &shieldOverHair;
    zmap[u"gloveWristBelowWeapon"] = &gloveWristBelowWeapon;
    zmap[u"gloveBelowWeapon"] = &gloveBelowWeapon;
    zmap[u"handBelowWeapon"] = &handBelowWeapon;
    zmap[u"weaponOverArm"] = &weaponOverArm;
    zmap[u"gloveWristBelowMailArm"] = &gloveWristBelowMailArm;
    zmap[u"mailArmOverHair"] = &mailArmOverHair;
    zmap[u"gloveBelowMailArm"] = &gloveBelowMailArm;
    zmap[u"armOverHair"] = &armOverHair;
    zmap[u"mailArmOverHairBelowWeapon"] = &mailArmOverHairBelowWeapon;
    zmap[u"armOverHairBelowWeapon"] = &armOverHairBelowWeapon;
    zmap[u"weaponBelowArm"] = &weaponBelowArm;
    zmap[u"capOverHair"] = &capOverHair;
    zmap[u"hairOverHead"] = &hairOverHead;
    zmap[u"accessoryEyeOverCap"] = &accessoryEyeOverCap;
    zmap[u"capAccessory"] = &capAccessory;
    zmap[u"cap"] = &cap;
    zmap[u"hair"] = &hair;
    zmap[u"accessoryEye"] = &accessoryEye;
    zmap[u"accessoryEyeShadow"] = &accessoryEyeShadow;
    zmap[u"accessoryFace"] = &accessoryFace;
    zmap[u"capAccessoryBelowAccFace"] = &capAccessoryBelowAccFace;
    zmap[u"accessoryEar"] = &accessoryEar;
    zmap[u"capBelowAccessory"] = &capBelowAccessory;
    zmap[u"accessoryFaceOverFaceBelowCap"] = &accessoryFaceOverFaceBelowCap;
    zmap[u"face"] = &face;
    zmap[u"accessoryFaceBelowFace"] = &accessoryFaceBelowFace;
    zmap[u"hairShade"] = &hairShade;
    zmap[u"head"] = &head;
    zmap[u"cape"] = &cape;
    zmap[u"gloveWrist"] = &gloveWrist;
    zmap[u"mailArm"] = &mailArm;
    zmap[u"glove"] = &glove;
    zmap[u"hand"] = &hand;
    zmap[u"arm"] = &arm;
    zmap[u"weapon"] = &weapon;
    zmap[u"shield"] = &shield;
    zmap[u"weaponOverArmBelowHead"] = &weaponOverArmBelowHead;
    zmap[u"gloveWristBelowHead"] = &gloveWristBelowHead;
    zmap[u"mailArmBelowHeadOverMailChest"] = &mailArmBelowHeadOverMailChest;
    zmap[u"gloveBelowHead"] = &gloveBelowHead;
    zmap[u"armBelowHeadOverMailChest"] = &armBelowHeadOverMailChest;
    zmap[u"mailArmBelowHead"] = &mailArmBelowHead;
    zmap[u"armBelowHead"] = &armBelowHead;
    zmap[u"weaponOverBody"] = &weaponOverBody;
    zmap[u"mailChestTop"] = &mailChestTop;
    zmap[u"gloveWristOverBody"] = &gloveWristOverBody;
    zmap[u"mailChestOverHighest"] = &mailChestOverHighest;
    zmap[u"pantsOverMailChest"] = &pantsOverMailChest;
    zmap[u"mailChest"] = &mailChest;
    zmap[u"shoesTop"] = &shoesTop;
    zmap[u"pantsOverShoesBelowMailChest"] = &pantsOverShoesBelowMailChest;
    zmap[u"shoesOverPants"] = &shoesOverPants;
    zmap[u"mailChestOverPants"] = &mailChestOverPants;
    zmap[u"pants"] = &pants;
    zmap[u"shoes"] = &shoes;
    zmap[u"pantsBelowShoes"] = &pantsBelowShoes;
    zmap[u"mailChestBelowPants"] = &mailChestBelowPants;
    zmap[u"gloveOverBody"] = &gloveOverBody;
    zmap[u"body"] = &body;
    zmap[u"gloveWristBelowBody"] = &gloveWristBelowBody;
    zmap[u"gloveBelowBody"] = &gloveBelowBody;
    zmap[u"capAccessoryBelowBody"] = &capAccessoryBelowBody;
    zmap[u"shieldBelowBody"] = &shieldBelowBody;
    zmap[u"capeBelowBody"] = &capeBelowBody;
    zmap[u"hairBelowBody"] = &hairBelowBody;
    zmap[u"weaponBelowBody"] = &weaponBelowBody;
    zmap[u"backHairOverCape"] = &backHairOverCape;
    zmap[u"backWing"] = &backWing;
    zmap[u"backWeaponOverShield"] = &backWeaponOverShield;
    zmap[u"backShield"] = &backShield;
    zmap[u"backCapOverHair"] = &backCapOverHair;
    zmap[u"backHair"] = &backHair;
    zmap[u"backCap"] = &backCap;
    zmap[u"backWeaponOverHead"] = &backWeaponOverHead;
    zmap[u"backHairBelowCapWide"] = &backHairBelowCapWide;
    zmap[u"backHairBelowCapNarrow"] = &backHairBelowCapNarrow;
    zmap[u"backHairBelowCap"] = &backHairBelowCap;
    zmap[u"backCape"] = &backCape;
    zmap[u"backAccessoryFaceOverHead"] = &backAccessoryFaceOverHead;
    zmap[u"backHead"] = &backHead;
    zmap[u"backMailChestOverPants"] = &backMailChestOverPants;
    zmap[u"backPantsOverMailChest"] = &backPantsOverMailChest;
    zmap[u"backMailChest"] = &backMailChest;
    zmap[u"backPantsOverShoesBelowMailChest"] = &backPantsOverShoesBelowMailChest;
    zmap[u"backShoes"] = &backShoes;
    zmap[u"backPants"] = &backPants;
    zmap[u"backShoesBelowPants"] = &backShoesBelowPants;
    zmap[u"backPantsBelowShoes"] = &backPantsBelowShoes;
    zmap[u"backMailChestBelowPants"] = &backMailChestBelowPants;
    zmap[u"backWeaponOverGlove"] = &backWeaponOverGlove;
    zmap[u"backGloveWrist"] = &backGloveWrist;
    zmap[u"backGlove"] = &backGlove;
    zmap[u"backBody"] = &backBody;
    zmap[u"backAccessoryEar"] = &backAccessoryEar;
    zmap[u"backAccessoryFace"] = &backAccessoryFace;
    zmap[u"backCapAccessory"] = &backCapAccessory;
    zmap[u"backMailChestAccessory"] = &backMailChestAccessory;
    zmap[u"backShieldBelowBody"] = &backShieldBelowBody;
    zmap[u"backHairBelowHead"] = &backHairBelowHead;
    zmap[u"backWeapon"] = &backWeapon;
    zmap[u"characterEnd"] = &characterEnd;
    zmap[u"saddleRear"] = &saddleRear;
    zmap[u"tamingMobRear"] = &tamingMobRear;
    zmap[u"mobEquipRear"] = &mobEquipRear;
    zmap[u"backMobEquipFront"] = &backMobEquipFront;
    zmap[u"backTamingMobFront"] = &backTamingMobFront;
    zmap[u"backMobEquipMid"] = &backMobEquipMid;
    zmap[u"backSaddle"] = &backSaddle;
    zmap[u"backMobEquipUnderSaddle"] = &backMobEquipUnderSaddle;
    zmap[u"backTamingMobMid"] = &backTamingMobMid;
}

void Character::init()
{
    character_node = Wz::Character->get_root();
    {
        auto body_node = character_node->find_from_path(u"00002000.img");
        auto head_node = character_node->find_from_path(u"00012000.img");
        for (auto stance_node : body_node->get_children())
        {
            auto ststr = stance_node.first;

            uint16_t attackdelay = 0;

            for (uint8_t frame = 0; auto frame_node = stance_node.second[0]->get_child(std::to_string(frame)); ++frame)
            {
                if (frame_node->get_child(u"action") != nullptr)
                {
                    uint8_t type = type_map.at(dynamic_cast<wz::Property<wz::wzstring> *>(frame_node->get_child(u"action"))->get());
                    uint8_t f = 0;
                    if (frame_node->get_child(u"frame"))
                    {
                        f = dynamic_cast<wz::Property<int> *>(frame_node->get_child(u"frame"))->get();
                    }
                    int delay = 100;
                    if (frame_node->get_child(u"delay"))
                    {
                        delay = dynamic_cast<wz::Property<int> *>(frame_node->get_child(u"delay"))->get();
                    }
                    body_actions[ststr][frame] = {type, f, delay};
                }
                else
                {
                    auto type = type_map.at(ststr);
                    auto delay_node = frame_node->get_child(u"delay");
                    auto delay = 100;
                    if (delay_node != nullptr)
                    {
                        delay = dynamic_cast<wz::Property<int> *>(delay_node)->get();
                    }
                    stance_delays[type][frame] = delay;

                    auto face_node = frame_node->get_child(u"face");
                    auto face = true;
                    if (face_node != nullptr)
                    {
                        face = dynamic_cast<wz::Property<unsigned short> *>(face_node)->get();
                    }
                    show_face[type][frame] = face;

                    std::unordered_map<Layer, std::unordered_map<std::u16string, SDL_FPoint>> body_shift_map;

                    for (auto part_node : frame_node->get_children())
                    {
                        auto part_name = part_node.first;

                        if (part_name != u"delay" && part_name != u"face")
                        {
                            auto part = part_node.second[0];
                            if (part->type == wz::Type::UOL)
                            {
                                part = dynamic_cast<wz::Property<wz::WzUOL> *>(part)->get_uol();
                            }

                            auto zstr = dynamic_cast<wz::Property<wz::wzstring> *>(part->get_child(u"z"))->get();
                            auto z = layer_map.at(zstr);

                            for (auto map_node : part->get_child(u"map")->get_children())
                            {
                                auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(map_node.second[0])->get();

                                body_shift_map[z].emplace(map_node.first, SDL_FPoint{(float)v.x, (float)v.y});
                            }
                        }
                    }
                    std::string frame_str = std::to_string(frame);
                    auto h = head_node->find_from_path(ststr + u"/" + std::u16string{frame_str.begin(), frame_str.end()} + u"/" + u"head");
                    if (h == nullptr)
                    {
                        h = head_node->find_from_path(u"front/head");
                    }
                    if (h->type == wz::Type::UOL)
                    {
                        h = dynamic_cast<wz::Property<wz::WzUOL> *>(h)->get_uol();
                    }
                    for (auto map_node : h->get_child(u"map")->get_children())
                    {
                        auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(map_node.second[0])->get();

                        body_shift_map[Layer::HEAD].emplace(map_node.first, SDL_FPoint{(float)v.x, (float)v.y});
                    }

                    body_positions[type][frame] = body_shift_map[Layer::BODY][u"navel"];

                    arm_positions[type][frame] = body_shift_map.count(Layer::ARM) ? (body_shift_map[Layer::ARM][u"hand"] - body_shift_map[Layer::ARM][u"navel"] + body_shift_map[Layer::BODY][u"navel"])
                                                                                  : (body_shift_map[Layer::ARM_OVER_HAIR][u"hand"] - body_shift_map[Layer::ARM_OVER_HAIR][u"navel"] + body_shift_map[Layer::BODY][u"navel"]);
                    hand_positions[type][frame] = body_shift_map[Layer::HAND_BELOW_WEAPON][u"handMove"];
                    head_positions[type][frame] = body_shift_map[Layer::BODY][u"neck"] - body_shift_map[Layer::HEAD][u"neck"];
                    face_positions[type][frame] = body_shift_map[Layer::BODY][u"neck"] - body_shift_map[Layer::HEAD][u"neck"] + body_shift_map[Layer::HEAD][u"brow"];
                    hair_positions[type][frame] = body_shift_map[Layer::HEAD][u"brow"] - body_shift_map[Layer::HEAD][u"neck"] + body_shift_map[Layer::BODY][u"neck"];
                }
            }
        }
    }
}

void Character::add_body(const std::u16string &val)
{
    auto body_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < Action::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            {
                auto sprw = Sprite::Wrap::load(body_node->find_from_path(type + u"/body"));
                SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                Transform f;
                auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                auto part = *zmap[z];
                part[i][no] = {f, sprw};
            }
            {
                if (body_node->find_from_path(type + u"/arm") != nullptr)
                {
                    auto arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(body_node->find_from_path(type + u"/arm/map/navel"))->get();
                    auto sprw = Sprite::Wrap::load(body_node->find_from_path(type + u"/arm"));
                    SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                    Transform f(body_positions[i][no] - SDL_FPoint{(float)arm_pos.x, (float)arm_pos.y});
                    auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                    auto part = *zmap[z];
                    part[i][no] = {f, sprw};
                }
            }
            {
                if (body_node->find_from_path(type + u"/hand") != nullptr)
                {
                    auto hand = body_node->find_from_path(type + u"/hand");
                    if (body_node->find_from_path(type + u"/hand/map/navel") != nullptr)
                    {
                        auto hand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(hand->find_from_path(u"map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(hand);
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)hand_pos.x, (float)hand_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
            {
                if (body_node->find_from_path(type + u"/lHand") != nullptr)
                {
                    auto lHand = body_node->find_from_path(type + u"/lHand");
                    if (lHand->find_from_path(u"map/handMove") != nullptr)
                    {
                        auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(lHand->find_from_path(u"map/handMove"))->get();
                        auto sprw = Sprite::Wrap::load(lHand);
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(hand_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                    else
                    {
                        auto lHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(lHand->find_from_path(u"map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(lHand);
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)lHand_pos.x, (float)lHand_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
            {
                if (body_node->find_from_path(type + u"/rHand") != nullptr)
                {
                    auto rHand = body_node->find_from_path(type + u"/rHand");
                    if (rHand->find_from_path(u"map/navel") != nullptr)
                    {
                        auto rHand_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(rHand->find_from_path(u"map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(rHand);
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)rHand_pos.x, (float)rHand_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
        }
    }
}

void Character::add_coat(const std::u16string &val)
{
    auto coat_node = character_node->find_from_path(u"Coat/" + val + u".img");
    if (coat_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                {
                    if (coat_node->find_from_path(type + u"/mail") != nullptr)
                    {
                        auto coat_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_node->find_from_path(type + u"/mail/map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(coat_node->find_from_path(type + u"/mail"));
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)coat_pos.x, (float)coat_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
                {
                    if (coat_node->find_from_path(type + u"/mailArm") != nullptr)
                    {
                        auto mail_arm_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(coat_node->find_from_path(type + u"/mailArm/map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(coat_node->find_from_path(type + u"/mailArm"));
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)mail_arm_pos.x, (float)mail_arm_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
        }
    }
}

void Character::add_cap(const std::u16string &val)
{
    auto cap_node = character_node->find_from_path(u"Cap/" + val + u".img");
    if (cap_node != nullptr)
    {
        cap_vslot.clear();
        auto vslot = dynamic_cast<wz::Property<wz::wzstring> *>(cap_node->find_from_path(u"info/vslot"))->get();
        for (size_t i = 0; i < vslot.size(); i += 2)
        {
            cap_vslot.emplace(vslot.substr(i, 2));
        }
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                cap_node = character_node->find_from_path(u"Cap/" + val + u".img/" + type);
                if (cap_node == nullptr)
                {
                    cap_node = character_node->find_from_path(u"Cap/" + val + u".img/default");
                }
                if (cap_node != nullptr)
                {
                    for (auto it : cap_node->get_children())
                    {
                        auto cap = it.second[0];
                        if (cap->type == wz::Type::UOL)
                        {
                            cap = dynamic_cast<wz::Property<wz::WzUOL> *>(cap)->get_uol();
                        }
                        auto cap_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(cap->find_from_path(u"map/brow"))->get();
                        auto sprw = Sprite::Wrap::load(cap);
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(face_positions[i][no] - SDL_FPoint{(float)cap_pos.x, (float)cap_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
        }
    }
}

void Character::add_pants(const std::u16string &val)
{
    auto pants_node = character_node->find_from_path(u"Pants/" + val + u".img");
    if (pants_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (pants_node->find_from_path(type + u"/pants") != nullptr)
                {
                    auto pants_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(pants_node->find_from_path(type + u"/pants/map/navel"))->get();
                    auto sprw = Sprite::Wrap::load(pants_node->find_from_path(type + u"/pants"));
                    SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                    Transform f(body_positions[i][no] - SDL_FPoint{(float)pants_pos.x, (float)pants_pos.y});
                    auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                    auto part = *zmap[z];
                    part[i][no] = {f, sprw};
                }
            }
        }
    }
}

void Character::add_head(const std::u16string &val)
{
    auto head_node = character_node->find_from_path(val + u".img");
    for (uint8_t i = 0; i < Action::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            if (head_node->find_from_path(type + u"/head") != nullptr)
            {
                auto sprw = Sprite::Wrap::load(head_node->find_from_path(type + u"/head"));
                SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                Transform f(head_positions[i][no]);
                auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                auto part = *zmap[z];
                part[i][no] = {f, sprw};
            }
            else
            {
                auto sprw = Sprite::Wrap::load(head_node->find_from_path(u"front/head"));
                SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                Transform f(head_positions[i][no]);
                auto z = u"head";
                auto part = *zmap[z];
                part[i][no] = {f, sprw};
            }
        }
    }
}

void Character::add_face(const std::u16string &val, std::u16string type, const std::u16string index)
{
    face_str = val;
    auto type_node = character_node->find_from_path(u"Face/" + val + u".img/" + type);
    auto canvas_node = type_node;
    if (index != u"")
    {
        canvas_node = type_node->find_from_path(index);
    }
    auto face_node = canvas_node->find_from_path(u"face");
    if (face_node != nullptr)
    {
        if (canvas_node->get_child(u"delay") != nullptr)
        {
            face_time = dynamic_cast<wz::Property<int> *>(canvas_node->get_child(u"delay"))->get();
        }
        face_max_index = type_node->children_count() - 1;
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto face_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(face_node->find_from_path(u"map/brow"))->get();
                auto sprw = Sprite::Wrap::load(face_node);
                SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                Transform f(face_positions[i][no] - SDL_FPoint{(float)face_pos.x, (float)face_pos.y});
                auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                auto part = *zmap[z];
                part[i][no] = {f, sprw};
            }
        }
    }
}

void Character::add_hairs(const std::u16string &val)
{
    for (uint8_t i = 0; i < Action::LENGTH; i++)
    {
        for (uint8_t no = 0; no < body_positions[i].size(); no++)
        {
            auto no_str = std::to_string(no);
            auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
            auto hairs_node = character_node->find_from_path(u"Hair/" + val + u".img/" + type);
            if (hairs_node == nullptr)
            {
                hairs_node = character_node->find_from_path(u"Hair/" + val + u".img/default");
            }
            for (auto it : hairs_node->get_children())
            {
                auto hairs = it.second[0];
                if (hairs->type == wz::Type::UOL)
                {
                    hairs = dynamic_cast<wz::Property<wz::WzUOL> *>(hairs)->get_uol();
                }
                if (it.first == u"hairShade")
                {
                    hairs = hairs->find_from_path(u"0");
                }
                auto hair_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(hairs->find_from_path(u"map/brow"))->get();
                auto sprw = Sprite::Wrap::load(hairs);
                SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                Transform f(face_positions[i][no] - SDL_FPoint{(float)hair_pos.x, (float)hair_pos.y});
                auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                auto part = *zmap[z];
                part[i][no] = {f, sprw};
            }
        }
    }
}

void Character::add_shoes(const std::u16string &val)
{
    auto shoes_node = character_node->find_from_path(u"Shoes/" + val + u".img");
    if (shoes_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (shoes_node->find_from_path(type + u"/shoes") != nullptr)
                {
                    auto shoes_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(shoes_node->find_from_path(type + u"/shoes/map/navel"))->get();
                    auto sprw = Sprite::Wrap::load(shoes_node->find_from_path(type + u"/shoes"));
                    SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                    Transform f(body_positions[i][no] - SDL_FPoint{(float)shoes_pos.x, (float)shoes_pos.y});
                    auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                    auto part = *zmap[z];
                    part[i][no] = {f, sprw};
                }
            }
        }
    }
}

void Character::add_weapon(const std::u16string &val)
{
    auto weapon_node = character_node->find_from_path(u"Weapon/" + val + u".img");
    if (weapon_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (weapon_node->find_from_path(type + u"/weapon") != nullptr)
                {
                    if (weapon_node->find_from_path(type + u"/weapon/map/hand") != nullptr)
                    {
                        auto weapon_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(weapon_node->find_from_path(type + u"/weapon/map/hand"))->get();
                        auto sprw = Sprite::Wrap::load(weapon_node->find_from_path(type + u"/weapon"));
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(arm_positions[i][no] - SDL_FPoint{(float)weapon_pos.x, (float)weapon_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                    else
                    {
                        auto weapon_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(weapon_node->find_from_path(type + u"/weapon/map/navel"))->get();
                        auto sprw = Sprite::Wrap::load(weapon_node->find_from_path(type + u"/weapon"));
                        SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                        Transform f(body_positions[i][no] - SDL_FPoint{(float)weapon_pos.x, (float)weapon_pos.y});
                        auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                        auto part = *zmap[z];
                        part[i][no] = {f, sprw};
                    }
                }
            }
        }
    }
}

void Character::add_shield(const std::u16string &val)
{
    auto shield_node = character_node->find_from_path(u"Shield/" + val + u".img");
    if (shield_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (shield_node->find_from_path(type + u"/shield") != nullptr)
                {
                    auto shield_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(shield_node->find_from_path(type + u"/shield/map/navel"))->get();
                    auto sprw = Sprite::Wrap::load(shield_node->find_from_path(type + u"/shield"));
                    SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                    Transform f(body_positions[i][no] - SDL_FPoint{(float)shield_pos.x, (float)shield_pos.y});
                    auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                    auto part = *zmap[z];
                    part[i][no] = {f, sprw};
                }
            }
        }
    }
}

void Character::add_cape(const std::u16string &val)
{
    auto cape_node = character_node->find_from_path(u"Cape/" + val + u".img");
    if (cape_node != nullptr)
    {
        for (uint8_t i = 0; i < Action::LENGTH; i++)
        {
            for (uint8_t no = 0; no < body_positions[i].size(); no++)
            {
                auto no_str = std::to_string(no);
                auto type = type_map2.at(i) + u"/" + std::u16string{no_str.begin(), no_str.end()};
                if (cape_node->find_from_path(type + u"/cape") != nullptr)
                {
                    auto shield_pos = dynamic_cast<wz::Property<wz::WzVec2D> *>(cape_node->find_from_path(type + u"/cape/map/navel"))->get();
                    auto sprw = Sprite::Wrap::load(cape_node->find_from_path(type + u"/cape"));
                    SDL_SetTextureScaleMode(sprw->texture, SDL_SCALEMODE_NEAREST);
                    Transform f(body_positions[i][no] - SDL_FPoint{(float)shield_pos.x, (float)shield_pos.y});
                    auto z = dynamic_cast<wz::Property<wz::wzstring> *>(sprw->n->get_child(u"z"))->get();
                    auto part = *zmap[z];
                    part[i][no] = {f, sprw};
                }
            }
        }
    }
}