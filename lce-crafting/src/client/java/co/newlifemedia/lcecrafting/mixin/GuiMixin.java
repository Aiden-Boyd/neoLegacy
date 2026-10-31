package co.newlifemedia.lcecrafting.mixin;
import co.newlifemedia.lcecrafting.LegacyCraftingScreen;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.client.gui.screens.inventory.CraftingScreen;
import net.minecraft.client.gui.screens.inventory.InventoryScreen;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.ModifyVariable;
@Mixin(Gui.class)
public abstract class GuiMixin {
    @ModifyVariable(method = "setScreen", at = @At("HEAD"), argsOnly = true)
    private Screen lce$craftingScreen(Screen screen) {
        var client = Minecraft.getInstance();
        if (client.player == null || LegacyCraftingScreen.bypassReplacement) return screen;
        if (screen instanceof CraftingScreen crafting) {
            return new LegacyCraftingScreen<>(crafting.getMenu(), client.player.getInventory());
        }
        if (screen instanceof InventoryScreen inventory && client.gameMode != null
                && !client.gameMode.isServerControlledInventory()) {
            return new LegacyCraftingScreen<>(inventory.getMenu(), client.player.getInventory());
        }
        return screen;
    }
}
