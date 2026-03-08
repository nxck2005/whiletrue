#!/usr/bin/env python3
import json
import os
import sys

def clear_screen():
    os.system('clear' if os.name == 'posix' else 'cls')

def main():
    upgrades_path = "data/upgrades.json"
    buildings_path = "data/buildings.json"

    if not os.path.exists(upgrades_path):
        print(f"Error: {upgrades_path} not found.")
        sys.exit(1)
    
    if not os.path.exists(buildings_path):
        print(f"Error: {buildings_path} not found.")
        sys.exit(1)

    with open(buildings_path, 'r') as f:
        buildings_list = json.load(f)
        buildings_map = {str(b['id']): b['name'] for b in buildings_list}

    with open(upgrades_path, 'r') as f:
        upgrades = json.load(f)

    while True:
        clear_screen()
        print("--- CYBERGRIND: SOFTWARE REPOSITORY ---")
        print(f"{'ID':<4} | {'Name':<25} | {'Cost':<12} | {'Visibility'}")
        print("-" * 60)
        
        for u in upgrades:
            vis = "Always" if u.get('alwaysVisible') else "Conditional"
            print(f"{u['id']:<4} | {u['name']:<25} | {u['cost']:<12,.0f} | {vis}")
        
        print("-" * 60)
        choice = input("\nEnter ID for details, 'r' to reload, or 'q' to quit: ").strip().lower()
        
        if choice == 'q':
            break
        elif choice == 'r':
            with open(upgrades_path, 'r') as f:
                upgrades = json.load(f)
            continue
        
        target = None
        for u in upgrades:
            if str(u['id']) == choice:
                target = u
                break
        
        if target:
            clear_screen()
            print(f"--- DETAILED VIEW: {target['name']} ---")
            print(f"ID:          {target['id']}")
            print(f"Description: {target['desc']}")
            print(f"Cost:        {target['cost']:,} DATA")
            print(f"Always Vis:  {target.get('alwaysVisible', False)}")
            print(f"Global Mult: {target.get('globalMultiplier', 1.0)}x")
            print(f"CPS Boost:   {target.get('cpsBoost', 0.0)*100:.1f}% of LPS to click")
            
            print("\nRequirements:")
            reqs = target.get('neededCountsBuildings', {})
            if not reqs:
                print("  None")
            for bid, count in reqs.items():
                bname = buildings_map.get(str(bid), f"Unknown({bid})")
                print(f"  - {bname}: {count}")
                
            print("\nBuilding Buffs:")
            buffs = target.get('resultantBuffs', {})
            if not buffs:
                print("  None")
            for bid, mult in buffs.items():
                bname = buildings_map.get(str(bid), f"Unknown({bid})")
                print(f"  - {bname}: {mult}x production")
            
            input("\nPress Enter to return to list...")
        elif choice != '':
            input("\nInvalid ID. Press Enter to continue...")

if __name__ == "__main__":
    main()
