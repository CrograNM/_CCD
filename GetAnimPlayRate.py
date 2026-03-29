
def GetAnimPlayRate (TargetSpeed, Location1, Time1, Location2, Time2):

    TargetAnimSpeed = TargetSpeed
    PlayerScale = 0.4225

    Distance = abs(Location2 - Location1)
    TimeDifference = Time2 - Time1
    if TimeDifference == 0:
        return 0
    
    AnimSpeed = Distance / TimeDifference
    print(f"Animation Speed: {AnimSpeed}")
    AnimPlayRate = TargetAnimSpeed / (AnimSpeed * PlayerScale)
    print(f"PlayRate for TargetSpeed: {AnimPlayRate}")

    return AnimPlayRate

# Example usage

print("\n=== Walk Forward ===")
PlayRate = GetAnimPlayRate(200, -70, 0.33, 120, 0.89)

print("\n=== Walk Backward ===")
PlayRate = GetAnimPlayRate(200, 20, 0.24, -120, 0.81)

print("\n=== Walk Left/Right ===")
PlayRate = GetAnimPlayRate(200, -80, 0.3, 110, 0.74)

print("\n=== Jog Backward ===")
PlayRate = GetAnimPlayRate(200, 10, 0.22, -100, 0.61)

print("\n=== Run Backward ===")
PlayRate = GetAnimPlayRate(500, 20, 0.24, -120, 0.43)