#!/usr/bin/env python3
import json
import os
import sys

def main():
    upgrades_path = "data/upgrades.json"
    buildings_path = "data/buildings.json"

    if not os.path.exists(upgrades_path):
        print(f"Error: {upgrades_path} not found.")
        sys.exit(1)
    
    if not os.path.exists(buildings_path):
        print(f"Error: {buildings_path} not found.")
        sys.exit(1)

    with open(upgrades_path, 'r') as f:
        try:
            upgrades = json.load(f)
        except json.JSONDecodeError:
            upgrades = []

    with open(buildings_path, 'r') as f:
        try:
            buildings_list = json.load(f)
            # Map building ID to name for reference
            buildings_map = {b['id']: b['name'] for b in buildings_list}
        except Exception as e:
            print(f"Error loading buildings: {e}")
            sys.exit(1)

    print("--- CYBERGRIND: SOFTWARE UPGRADE GENERATOR ---")
    
    # Calculate next ID
    next_id = 0
    if upgrades:
        next_id = max(u['id'] for u in upgrades) + 1
    
    name = input(f"Upgrade Name: ").strip()
    desc = input(f"Description: ").strip()
    cost = float(input(f"Cost (DATA): ") or 0.0)
    always_visible = input("Always Visible? (y/n, default n): ").lower() == 'y'
    global_mult = float(input("Global Multiplier (e.g. 1.2 for +20%, default 1.0): ") or 1.0)

    # Requirements
    needed_counts = {}
    print("\n--- REQUIREMENTS ---")
    print("Available Buildings:")
    for bid, bname in buildings_map.items():
        print(f"  [{bid}] {bname}")
    
    while True:
        bid = input("Add required building ID (or leave blank to finish): ").strip()
        if not bid:
            break
        try:
            bid_int = int(bid)
        except ValueError:
            print("Invalid ID (must be integer)!")
            continue
            
        if bid_int not in buildings_map:
            print("Invalid building ID!")
            continue
        count = int(input(f"  Count for {buildings_map[bid_int]}: "))
        needed_counts[bid] = count

    # Buffs
    resultant_buffs = {}
    print("\n--- BUILDING BUFFS ---")
    while True:
        bid = input("Add buff for building ID (or leave blank to finish): ").strip()
        if not bid:
            break
        try:
            bid_int = int(bid)
        except ValueError:
            print("Invalid ID (must be integer)!")
            continue

        if bid_int not in buildings_map:
            print("Invalid building ID!")
            continue
        mult = float(input(f"  Multiplier for {buildings_map[bid_int]} (e.g. 2.0 for double): "))
        resultant_buffs[bid] = mult

    new_upgrade = {
        "id": next_id,
        "name": name,
        "desc": desc,
        "cost": cost,
        "alwaysVisible": always_visible,
        "neededCountsBuildings": needed_counts,
        "resultantBuffs": resultant_buffs,
        "globalMultiplier": global_mult
    }

    print("\n--- PREVIEW ---")
    print(json.dumps(new_upgrade, indent=2))
    
    confirm = input("\nAppend this upgrade to upgrades.json? (y/n): ").lower()
    if confirm == 'y':
        upgrades.append(new_upgrade)
        with open(upgrades_path, 'w') as f:
            json.dump(upgrades, f, indent=2)
        print(f"Successfully added '{name}' with ID {next_id}.")
    else:
        print("Aborted.")

if __name__ == "__main__":
    main()
